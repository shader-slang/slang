// unit-test-nvvm-compiler.cpp

#include "compiler-core/slang-artifact-representation.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-nvvm-compiler.h"
#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-shared-library.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

namespace
{

// These declarations mirror the small public libNVVM C ABI used by the production implementation.
// Keeping the fake independent of CUDA headers verifies that libNVVM remains an optional build
// dependency.
enum class TestNVVMResult
{
    Success = 0,
    OutOfMemory = 1,
    ProgramCreationFailure = 2,
    IRVersionMismatch = 3,
    InvalidInput = 4,
    InvalidProgram = 5,
    InvalidIR = 6,
    InvalidOption = 7,
    NoModuleInProgram = 8,
    Compilation = 9,
    Cancelled = 10,
};

struct TestNVVMProgramStorage
{
    int unused;
};
typedef TestNVVMProgramStorage* TestNVVMProgram;

enum class FakeFailure
{
    None,
    CreateProgram,
    AddModule,
    VerifyProgram,
    CompileProgram,
    GetResultSize,
    GetResult,
    GetLogSize,
    GetLog,
};

enum class FakeLogPhase
{
    General,
    Verifier,
    Compiler,
};

enum class FakeResultMode
{
    NullTerminated,
    TerminatorOnly,
    Unterminated,
};

static const char* const kRequiredSymbols[] = {
    "nvvmGetErrorString",
    "nvvmVersion",
    "nvvmIRVersion",
    "nvvmCreateProgram",
    "nvvmDestroyProgram",
    "nvvmAddModuleToProgram",
    "nvvmVerifyProgram",
    "nvvmCompileProgram",
    "nvvmGetCompiledResultSize",
    "nvvmGetCompiledResult",
    "nvvmGetProgramLogSize",
    "nvvmGetProgramLog",
};

static const char kMinimalNVVMIR[] =
    "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
    "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
    "v128:128:128-n16:32:64\"\n"
    "target triple = \"nvptx64-nvidia-cuda\"\n"
    "\n"
    "define void @testEmpty() {\n"
    "entry:\n"
    "  ret void\n"
    "}\n"
    "\n"
    "!nvvmir.version = !{!0}\n"
    "!nvvm.annotations = !{!1}\n"
    "!0 = !{i32 2, i32 0}\n"
    "!1 = !{void ()* @testEmpty, !\"kernel\", i32 1}\n";

static const char kFakePTX[] = ".version 7.5\n"
                               ".target sm_75\n"
                               ".address_size 64\n"
                               ".visible .entry testEmpty()\n"
                               "{\n"
                               "    ret;\n"
                               "}\n";

struct FakeNVVMState
{
    void resetCalls()
    {
        createProgramCallCount = 0;
        destroyProgramCallCount = 0;
        addModuleCallCount = 0;
        verifyProgramCallCount = 0;
        compileProgramCallCount = 0;
        getResultSizeCallCount = 0;
        getResultCallCount = 0;
        getLogSizeCallCount = 0;
        getLogCallCount = 0;
        addedModule = String();
        addedModuleName = String();
        verifyOptions.clear();
        compileOptions.clear();
        currentLogPhase = FakeLogPhase::General;
    }

    void reset()
    {
        SLANG_ASSERT(liveLibraryCount == 0);
        failure = FakeFailure::None;
        failureResult = TestNVVMResult::Compilation;
        missingSymbol = String();
        omitOptionalSymbols = false;
        loadedPath = String();
        successfulLoadCount = 0;
        liveLibraryCount = 0;
        destroyedLibraryCount = 0;
        programLog = String();
        verifierLog = String();
        compilerLog = String();
        usePhaseLogs = false;
        resultMode = FakeResultMode::NullTerminated;
        compiledPTX = kFakePTX;
        resetCalls();
    }

    FakeFailure failure = FakeFailure::None;
    TestNVVMResult failureResult = TestNVVMResult::Compilation;
    String missingSymbol;
    bool omitOptionalSymbols = false;

    String loadedPath;
    int successfulLoadCount = 0;
    int liveLibraryCount = 0;
    int destroyedLibraryCount = 0;

    TestNVVMProgramStorage programStorage = {};
    int createProgramCallCount = 0;
    int destroyProgramCallCount = 0;
    int addModuleCallCount = 0;
    int verifyProgramCallCount = 0;
    int compileProgramCallCount = 0;
    int getResultSizeCallCount = 0;
    int getResultCallCount = 0;
    int getLogSizeCallCount = 0;
    int getLogCallCount = 0;

    String addedModule;
    String addedModuleName;
    List<String> verifyOptions;
    List<String> compileOptions;
    String programLog;
    String verifierLog;
    String compilerLog;
    FakeLogPhase currentLogPhase = FakeLogPhase::General;
    bool usePhaseLogs = false;
    FakeResultMode resultMode = FakeResultMode::NullTerminated;
    String compiledPTX;
};

FakeNVVMState gFakeNVVM;

static TestNVVMResult _fakeFailureResult(FakeFailure operation)
{
    return gFakeNVVM.failure == operation ? gFakeNVVM.failureResult : TestNVVMResult::Success;
}

static bool _isFakeProgram(TestNVVMProgram program)
{
    return program == &gFakeNVVM.programStorage;
}

static const String& _getCurrentFakeProgramLog()
{
    if (gFakeNVVM.usePhaseLogs)
    {
        switch (gFakeNVVM.currentLogPhase)
        {
        case FakeLogPhase::Verifier:
            return gFakeNVVM.verifierLog;
        case FakeLogPhase::Compiler:
            return gFakeNVVM.compilerLog;
        default:
            break;
        }
    }
    return gFakeNVVM.programLog;
}

static void _captureOptions(int optionCount, const char** options, List<String>& outOptions)
{
    outOptions.clear();
    for (int i = 0; i < optionCount; ++i)
        outOptions.add(options[i]);
}

static const char* _fakeGetErrorString(TestNVVMResult result)
{
    return result == TestNVVMResult::Success ? "success" : "fake NVVM compilation failure";
}

static TestNVVMResult _fakeVersion(int* major, int* minor)
{
    if (!major || !minor)
        return TestNVVMResult::InvalidInput;
    *major = 2;
    *minor = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeIRVersion(int* majorIR, int* minorIR, int* majorDebug, int* minorDebug)
{
    if (!majorIR || !minorIR || !majorDebug || !minorDebug)
        return TestNVVMResult::InvalidInput;
    *majorIR = 2;
    *minorIR = 0;
    *majorDebug = 3;
    *minorDebug = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeCreateProgram(TestNVVMProgram* outProgram)
{
    ++gFakeNVVM.createProgramCallCount;
    if (!outProgram)
        return TestNVVMResult::InvalidInput;
    if (gFakeNVVM.failure == FakeFailure::CreateProgram)
        return TestNVVMResult::ProgramCreationFailure;
    *outProgram = &gFakeNVVM.programStorage;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeDestroyProgram(TestNVVMProgram* program)
{
    if (!program || !_isFakeProgram(*program))
        return TestNVVMResult::InvalidProgram;
    ++gFakeNVVM.destroyProgramCallCount;
    *program = nullptr;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeAddModuleToProgram(
    TestNVVMProgram program,
    const char* buffer,
    size_t size,
    const char* name)
{
    ++gFakeNVVM.addModuleCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::General;
    if (!_isFakeProgram(program) || (!buffer && size) || !name)
        return TestNVVMResult::InvalidInput;
    gFakeNVVM.addedModule = String(UnownedStringSlice(buffer, size));
    gFakeNVVM.addedModuleName = name;
    return _fakeFailureResult(FakeFailure::AddModule);
}

static TestNVVMResult _fakeVerifyProgram(
    TestNVVMProgram program,
    int optionCount,
    const char** options)
{
    ++gFakeNVVM.verifyProgramCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::Verifier;
    if (!_isFakeProgram(program) || optionCount < 0 || (optionCount && !options))
        return TestNVVMResult::InvalidInput;
    _captureOptions(optionCount, options, gFakeNVVM.verifyOptions);
    return _fakeFailureResult(FakeFailure::VerifyProgram);
}

static TestNVVMResult _fakeCompileProgram(
    TestNVVMProgram program,
    int optionCount,
    const char** options)
{
    ++gFakeNVVM.compileProgramCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::Compiler;
    if (!_isFakeProgram(program) || optionCount < 0 || (optionCount && !options))
        return TestNVVMResult::InvalidInput;
    _captureOptions(optionCount, options, gFakeNVVM.compileOptions);
    return _fakeFailureResult(FakeFailure::CompileProgram);
}

static TestNVVMResult _fakeGetCompiledResultSize(TestNVVMProgram program, size_t* outSize)
{
    ++gFakeNVVM.getResultSizeCallCount;
    if (!_isFakeProgram(program) || !outSize)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetResultSize);
    if (result != TestNVVMResult::Success)
        return result;
    // A conforming libNVVM result includes the C-string terminator in its reported size. The other
    // modes deliberately violate that contract so the compiler's boundary checks can be tested.
    switch (gFakeNVVM.resultMode)
    {
    case FakeResultMode::TerminatorOnly:
        *outSize = 1;
        break;
    case FakeResultMode::Unterminated:
        *outSize = size_t(gFakeNVVM.compiledPTX.getLength());
        break;
    default:
        *outSize = size_t(gFakeNVVM.compiledPTX.getLength()) + 1;
        break;
    }
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetCompiledResult(TestNVVMProgram program, char* outResult)
{
    ++gFakeNVVM.getResultCallCount;
    if (!_isFakeProgram(program) || !outResult)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetResult);
    if (result != TestNVVMResult::Success)
        return result;
    if (gFakeNVVM.resultMode == FakeResultMode::TerminatorOnly)
    {
        outResult[0] = 0;
        return TestNVVMResult::Success;
    }
    const Index size = gFakeNVVM.compiledPTX.getLength();
    if (size)
        ::memcpy(outResult, gFakeNVVM.compiledPTX.getBuffer(), size_t(size));
    if (gFakeNVVM.resultMode == FakeResultMode::NullTerminated)
        outResult[size] = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetProgramLogSize(TestNVVMProgram program, size_t* outSize)
{
    ++gFakeNVVM.getLogSizeCallCount;
    if (!_isFakeProgram(program) || !outSize)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetLogSize);
    if (result != TestNVVMResult::Success)
        return result;
    *outSize = size_t(_getCurrentFakeProgramLog().getLength()) + 1;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetProgramLog(TestNVVMProgram program, char* outLog)
{
    ++gFakeNVVM.getLogCallCount;
    if (!_isFakeProgram(program) || !outLog)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetLog);
    if (result != TestNVVMResult::Success)
        return result;
    const String& programLog = _getCurrentFakeProgramLog();
    const Index size = programLog.getLength();
    if (size)
        ::memcpy(outLog, programLog.getBuffer(), size_t(size));
    outLog[size] = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeLazyAddModuleToProgram(
    TestNVVMProgram program,
    const char* buffer,
    size_t size,
    const char* name)
{
    SLANG_UNUSED(program);
    SLANG_UNUSED(buffer);
    SLANG_UNUSED(size);
    SLANG_UNUSED(name);
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeLLVMVersion(const char* architecture, int* major)
{
    SLANG_UNUSED(architecture);
    if (major)
        *major = 7;
    return TestNVVMResult::Success;
}

class FakeNVVMLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    FakeNVVMLibrary() { ++gFakeNVVM.liveLibraryCount; }

    ~FakeNVVMLibrary()
    {
        --gFakeNVVM.liveLibraryCount;
        ++gFakeNVVM.destroyedLibraryCount;
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(const char* name)
        SLANG_OVERRIDE
    {
        if (!name)
            return nullptr;
        const UnownedStringSlice symbol(name);
        if (gFakeNVVM.missingSymbol.getLength() &&
            symbol == gFakeNVVM.missingSymbol.getUnownedSlice())
        {
            return nullptr;
        }

        if (symbol == "nvvmGetErrorString")
            return (void*)_fakeGetErrorString;
        if (symbol == "nvvmVersion")
            return (void*)_fakeVersion;
        if (symbol == "nvvmIRVersion")
            return (void*)_fakeIRVersion;
        if (symbol == "nvvmCreateProgram")
            return (void*)_fakeCreateProgram;
        if (symbol == "nvvmDestroyProgram")
            return (void*)_fakeDestroyProgram;
        if (symbol == "nvvmAddModuleToProgram")
            return (void*)_fakeAddModuleToProgram;
        if (symbol == "nvvmVerifyProgram")
            return (void*)_fakeVerifyProgram;
        if (symbol == "nvvmCompileProgram")
            return (void*)_fakeCompileProgram;
        if (symbol == "nvvmGetCompiledResultSize")
            return (void*)_fakeGetCompiledResultSize;
        if (symbol == "nvvmGetCompiledResult")
            return (void*)_fakeGetCompiledResult;
        if (symbol == "nvvmGetProgramLogSize")
            return (void*)_fakeGetProgramLogSize;
        if (symbol == "nvvmGetProgramLog")
            return (void*)_fakeGetProgramLog;
        if (!gFakeNVVM.omitOptionalSymbols && symbol == "nvvmLazyAddModuleToProgram")
            return (void*)_fakeLazyAddModuleToProgram;
        if (!gFakeNVVM.omitOptionalSymbols && symbol == "nvvmLLVMVersion")
            return (void*)_fakeLLVMVersion;
        return nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
};

class FakeNVVMLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;
        gFakeNVVM.loadedPath = path ? path : "";
        if (!path || UnownedStringSlice(path) != "nvvm")
            return SLANG_E_NOT_FOUND;

        ++gFakeNVVM.successfulLoadCount;
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMLibrary);
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

// Records filesystem load spellings while returning the in-process fake library. The candidate
// files used by discovery tests are inert; this loader ensures none reaches the platform loader.
class RecordingFakeNVVMLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!path || !outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;
        loadRequests.add(path);
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMLibrary);
        *outLibrary = library.detach();
        return SLANG_OK;
    }

    List<String> loadRequests;

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

static IDownstreamCompiler* _findNVVMCompiler(DownstreamCompilerSet* set)
{
    DownstreamCompilerDesc desc(SLANG_PASS_THROUGH_NVVM);
    return DownstreamCompilerUtil::findCompiler(
        set,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

static SlangResult _locateFakeNVVM(
    RefPtr<DownstreamCompilerSet>& outSet,
    IDownstreamCompiler*& outCompiler)
{
    outSet = new DownstreamCompilerSet;
    outCompiler = nullptr;
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
    SLANG_RETURN_ON_FAIL(NVVMDownstreamCompilerUtil::locateCompilers(String(), loader, outSet));
    outCompiler = _findNVVMCompiler(outSet);
    return outCompiler ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _locateRealNVVM(
    const String& path,
    RefPtr<DownstreamCompilerSet>& outSet,
    IDownstreamCompiler*& outCompiler)
{
    outSet = new DownstreamCompilerSet;
    outCompiler = nullptr;
    SLANG_RETURN_ON_FAIL(NVVMDownstreamCompilerUtil::locateCompilers(
        path,
        DefaultSharedLibraryLoader::getSingleton(),
        outSet));
    outCompiler = _findNVVMCompiler(outSet);
    return outCompiler ? SLANG_OK : SLANG_FAIL;
}

static ComPtr<IArtifact> _createNVVMIRArtifact(const char* ir = kMinimalNVVMIR)
{
    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_SHADER_LLVM_IR);
    artifact->addRepresentationUnknown(StringBlob::create(UnownedStringSlice(ir)));
    return artifact;
}

struct CompileSettings
{
    DownstreamCompileOptions::OptimizationLevel optimizationLevel =
        DownstreamCompileOptions::OptimizationLevel::Default;
    DownstreamCompileOptions::DebugInfoType debugInfoType =
        DownstreamCompileOptions::DebugInfoType::None;
    DownstreamCompileOptions::FloatingPointMode floatingPointMode =
        DownstreamCompileOptions::FloatingPointMode::Default;
    DownstreamCompileOptions::FloatingPointDenormalMode denormalModeFp32 =
        DownstreamCompileOptions::FloatingPointDenormalMode::Any;
    bool addFakeCompilerArgument = false;
};

static SlangResult _compileNVVM(
    IDownstreamCompiler* compiler,
    IArtifact* sourceArtifact,
    const CompileSettings& settings,
    IArtifact** outArtifact)
{
    IArtifact* sourceArtifacts[] = {sourceArtifact};
    DownstreamCompileOptions::CapabilityVersion capability;
    capability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    capability.version.set(7, 5);
    TerminatedCharSlice fakeArgument("-fake-nvvm-option");

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    options.targetType = SLANG_PTX;
    options.optimizationLevel = settings.optimizationLevel;
    options.debugInfoType = settings.debugInfoType;
    options.floatingPointMode = settings.floatingPointMode;
    options.denormalModeFp32 = settings.denormalModeFp32;
    options.sourceArtifacts = makeSlice(sourceArtifacts, SLANG_COUNT_OF(sourceArtifacts));
    options.requiredCapabilityVersions = makeSlice(&capability, 1);
    if (settings.addFakeCompilerArgument)
        options.compilerSpecificArguments = makeSlice(&fakeArgument, 1);
    return compiler->compile(options, outArtifact);
}

static bool _hasOption(const List<String>& options, const char* expected)
{
    for (const auto& option : options)
    {
        if (option == expected)
            return true;
    }
    return false;
}

static bool _diagnosticsContain(IArtifactDiagnostics* diagnostics, const char* expected)
{
    if (!diagnostics)
        return false;
    const TerminatedCharSlice raw = diagnostics->getRaw();
    return raw.data && ::strstr(raw.data, expected);
}

static IArtifactDiagnostics* _findDiagnostics(IArtifact* artifact)
{
    return artifact ? findAssociatedRepresentation<IArtifactDiagnostics>(artifact) : nullptr;
}

static void _reportArtifactDiagnostics(IArtifact* artifact)
{
    IArtifactDiagnostics* diagnostics = _findDiagnostics(artifact);
    if (!diagnostics)
        return;
    const TerminatedCharSlice raw = diagnostics->getRaw();
    if (raw.count)
        getTestReporter()->message(TestMessageType::Info, raw.data);
}

struct TempPtxasOutput
{
    String lockPath;
    String cubinPath;

    ~TempPtxasOutput()
    {
        if (cubinPath.getLength())
            File::remove(cubinPath);
        if (lockPath.getLength())
            File::remove(lockPath);
    }
};

struct TempDirectory
{
    String path;

    ~TempDirectory()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

static SlangResult _createTempDirectory(TempDirectory& outDirectory)
{
    SLANG_RETURN_ON_FAIL(
        File::generateTemporary(toSlice("slang-nvvm-discovery"), outDirectory.path));
    SLANG_RETURN_ON_FAIL(File::remove(outDirectory.path));
    if (!Path::createDirectoryRecursive(outDirectory.path))
        return SLANG_FAIL;
    return SLANG_OK;
}

static void _checkRejectedCompiledResult(FakeResultMode resultMode)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.resultMode = resultMode;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    const SlangResult result =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);

    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(diagnostics->getCount() >= 1);
    SLANG_CHECK(outputArtifact->getRepresentations().count == 0);
    SLANG_CHECK(gFakeNVVM.getResultSizeCallCount == 1);
    SLANG_CHECK(gFakeNVVM.getResultCallCount == 1);
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
}

static void _checkRejectedInputResult(SlangResult result, IArtifact* artifact)
{
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK_ABORT(artifact != nullptr);
    SLANG_CHECK(
        artifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics = _findDiagnostics(artifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(diagnostics->getCount() >= 1);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

} // namespace

// Exercise the public lazy-discovery path, not just the locator in isolation. This catches a new
// pass-through enum being added without registering its default downstream compiler locator.
SLANG_UNIT_TEST(nvvmPassThroughDiscoversInjectedLibrary)
{
    gFakeNVVM.reset();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        SLANG_CHECK(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM) == SLANG_OK);
        int major = -1;
        int minor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NVVM, &major, &minor) ==
            SLANG_OK);
        SLANG_CHECK(major == 2);
        SLANG_CHECK(minor == 0);
        SLANG_CHECK(gFakeNVVM.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVM.loadedPath == "nvvm");
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRejectsMissingRequiredSymbols)
{
    gFakeNVVM.reset();
    for (const char* missingSymbol : kRequiredSymbols)
    {
        gFakeNVVM.missingSymbol = missingSymbol;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
            RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
            SlangResult result = NVVMDownstreamCompilerUtil::locateCompilers(String(), loader, set);
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(result != SLANG_E_NOT_FOUND);
            SLANG_CHECK(!set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
        }
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmLocatorAcceptsMissingOptionalSymbols)
{
    gFakeNVVM.reset();
    gFakeNVVM.omitOptionalSymbols = true;
    {
        RefPtr<DownstreamCompilerSet> set;
        IDownstreamCompiler* compiler = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
        SLANG_CHECK(compiler != nullptr);
        SLANG_CHECK(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);
        SLANG_CHECK(compiler->getDesc().version == SemanticVersion(2, 0));
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRanksNumericCandidates)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String lowerPath = Path::combine(tempDirectory.path, "nvvm64_90_0.dll");
    const String higherPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(higherPath);
#else
    const String lowerPath = Path::combine(tempDirectory.path, "libnvvm.so.9");
    const String higherPath = Path::combine(tempDirectory.path, "libnvvm.so.10");
    const String expectedLoadPath = higherPath;
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(lowerPath, String())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(higherPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        NVVMDownstreamCompilerUtil::locateCompilers(tempDirectory.path, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmLocatorNormalizesDecoratedExplicitFile)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(decoratedPath);
#elif SLANG_LINUX_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.so");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#else
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.dylib");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(decoratedPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(NVVMDownstreamCompilerUtil::locateCompilers(decoratedPath, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmCompilerOwnsLibrary)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* foundCompiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, foundCompiler)));
    ComPtr<IDownstreamCompiler> compiler(foundCompiler);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);

    set.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);
    compiler.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsInvalidInputs)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<IArtifact> validArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> wrongArtifact =
        ArtifactUtil::createArtifactForCompileTarget(SLANG_HOST_LLVM_IR);
    wrongArtifact->addRepresentationUnknown(StringBlob::create(UnownedStringSlice(kMinimalNVVMIR)));

    IArtifact* oneValidSource[] = {validArtifact};
    IArtifact* twoValidSources[] = {validArtifact, validArtifact};
    IArtifact* oneWrongSource[] = {wrongArtifact};
    DownstreamCompileOptions::CapabilityVersion validCapability;
    validCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    validCapability.version.set(7, 5);
    DownstreamCompileOptions::CapabilityVersion malformedCapability;
    malformedCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    malformedCapability.version.set(7, 10);

    DownstreamCompileOptions baseOptions;
    baseOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    baseOptions.targetType = SLANG_PTX;
    baseOptions.debugInfoType = DownstreamCompileOptions::DebugInfoType::None;

    // No source artifacts.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // More than one source artifact.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(twoValidSources, SLANG_COUNT_OF(twoValidSources));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // LLVM IR for the host has the right payload but the wrong artifact style.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneWrongSource, SLANG_COUNT_OF(oneWrongSource));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // A valid source still requires an explicit CUDA architecture capability.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // CUDA architecture minor versions contain one decimal digit.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        options.requiredCapabilityVersions = makeSlice(&malformedCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }
}

SLANG_UNIT_TEST(nvvmCompilerCompilesTrivialIR)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<slang::IBlob> versionString;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->getVersionString(versionString.writeRef())));
    String version(UnownedStringSlice(
        (const char*)versionString->getBufferPointer(),
        versionString->getBufferSize()));
    SLANG_CHECK(version.indexOf("2.0") >= 0);
    SLANG_CHECK(version.indexOf("nvvm-ir=2.0") >= 0);
    SLANG_CHECK(version.indexOf("debug=3.0") >= 0);

    CompileSettings settings;
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None;
    settings.debugInfoType = DownstreamCompileOptions::DebugInfoType::Maximal;
    settings.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Precise;
    settings.denormalModeFp32 = DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
    settings.addFakeCompilerArgument = true;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);

    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_OK);

    ComPtr<ISlangBlob> outputBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, outputBlob.writeRef())));
    SLANG_CHECK(outputBlob->getBufferSize() == ::strlen(kFakePTX));
    SLANG_CHECK(::memcmp(outputBlob->getBufferPointer(), kFakePTX, ::strlen(kFakePTX)) == 0);
    if (outputBlob->getBufferSize())
    {
        const char* bytes = (const char*)outputBlob->getBufferPointer();
        SLANG_CHECK(bytes[outputBlob->getBufferSize() - 1] != 0);
    }

    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.addedModule == kMinimalNVVMIR);
    SLANG_CHECK(gFakeNVVM.addedModuleName == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.verifyOptions.getCount() == 8);
    SLANG_CHECK(gFakeNVVM.compileOptions.getCount() == 8);
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-arch=compute_75"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-g"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-opt=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-ftz=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-div=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-sqrt=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fma=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fake-nvvm-option"));
    for (Index i = 0; i < gFakeNVVM.compileOptions.getCount(); ++i)
        SLANG_CHECK(gFakeNVVM.verifyOptions[i] == gFakeNVVM.compileOptions[i]);

    // Maximal debug metadata is only valid for unoptimized code. Reject the combination before
    // creating a libNVVM program so the policy cannot be silently weakened by option ordering.
    gFakeNVVM.resetCalls();
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;
    ComPtr<IArtifact> invalidOutput;
    SlangResult invalidResult =
        _compileNVVM(compiler, sourceArtifact, settings, invalidOutput.writeRef());
    SLANG_CHECK(invalidResult == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(invalidOutput != nullptr);
    diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(invalidOutput);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_E_INVALID_ARG);
    SLANG_CHECK(_diagnosticsContain(diagnostics, "requires optimization to be disabled"));
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsTerminatorOnlyResult)
{
    _checkRejectedCompiledResult(FakeResultMode::TerminatorOnly);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsUnterminatedResult)
{
    _checkRejectedCompiledResult(FakeResultMode::Unterminated);
}

SLANG_UNIT_TEST(nvvmCompilerDestroysProgramsOnFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    static const FakeFailure kFailures[] = {
        FakeFailure::CreateProgram,
        FakeFailure::AddModule,
        FakeFailure::VerifyProgram,
        FakeFailure::CompileProgram,
        FakeFailure::GetResultSize,
        FakeFailure::GetResult,
        FakeFailure::GetLogSize,
        FakeFailure::GetLog,
    };
    for (FakeFailure failure : kFailures)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failure;
        ComPtr<IArtifact> outputArtifact;
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics =
            findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
        SLANG_CHECK(
            gFakeNVVM.destroyProgramCallCount == (failure == FakeFailure::CreateProgram ? 0 : 1));
    }
    gFakeNVVM.failure = FakeFailure::None;
}

SLANG_UNIT_TEST(nvvmCompilerClassifiesVerificationAndCompilationFailures)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    struct FailureCase
    {
        FakeFailure operation;
        TestNVVMResult nvvmResult;
        SlangResult callResult;
        SlangResult diagnosticResult;
    };
    static const FailureCase kCases[] = {
        {FakeFailure::VerifyProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::CompileProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::VerifyProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::CompileProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::VerifyProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
        {FakeFailure::CompileProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
    };

    for (const auto& failureCase : kCases)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failureCase.operation;
        gFakeNVVM.failureResult = failureCase.nvvmResult;
        ComPtr<IArtifact> outputArtifact;
        SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK(result == failureCase.callResult);
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(diagnostics->getResult() == failureCase.diagnosticResult);
        SLANG_CHECK(diagnostics->getCount() >= 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    }
    gFakeNVVM.failure = FakeFailure::None;
    gFakeNVVM.failureResult = TestNVVMResult::Compilation;
}

SLANG_UNIT_TEST(nvvmCompilerUsesErrorStringForEmptyLog)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.failure = FakeFailure::VerifyProgram;
    gFakeNVVM.programLog = String();

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    // Verification/compilation failures are represented on the artifact so the caller can consume
    // libNVVM's diagnostics through the same channel as other downstream compilers.
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "libNVVM verification failed"));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "fake NVVM compilation failure"));
    SLANG_CHECK(diagnostics->getCount() >= 1);
}

SLANG_UNIT_TEST(nvvmCompilerPreservesVerifierLogOnCompilationFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.usePhaseLogs = true;
    gFakeNVVM.verifierLog = "fake verifier success note";
    gFakeNVVM.compilerLog = "fake compiler failure detail";
    gFakeNVVM.failure = FakeFailure::CompileProgram;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));

    const TerminatedCharSlice raw = diagnostics->getRaw();
    const String rawText(UnownedStringSlice(raw.data, raw.count));
    const Index verifierLogIndex = rawText.indexOf(gFakeNVVM.verifierLog);
    const Index compilerLogIndex = rawText.indexOf(gFakeNVVM.compilerLog);
    SLANG_CHECK(verifierLogIndex >= 0);
    SLANG_CHECK(compilerLogIndex > verifierLogIndex);
    SLANG_CHECK(rawText.indexOf("libNVVM compilation failed") > verifierLogIndex);
}

SLANG_UNIT_TEST(nvvmCompilerCompilesEmptyKernel)
{
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(String(), set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libNVVM smoke test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outputArtifact);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));
    String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    SLANG_CHECK(ptx.indexOf(".visible .entry testEmpty") >= 0);
    SLANG_CHECK(ptxBlob->getBufferSize() > 0);
    if (ptxBlob->getBufferSize())
    {
        const char* bytes = (const char*)ptxBlob->getBufferPointer();
        SLANG_CHECK(bytes[ptxBlob->getBufferSize() - 1] != 0);
    }
}

SLANG_UNIT_TEST(nvvmPtxasAcceptsEmptyKernel)
{
    StringBuilder cudaRootBuilder;
    if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(toSlice("CUDA_PATH"), cudaRootBuilder)) ||
        !cudaRootBuilder.getLength())
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH is not set.");
        SLANG_IGNORE_TEST;
    }
    const String cudaRoot = cudaRootBuilder.produceString();
    const String ptxasPath = Path::combine(
        Path::combine(cudaRoot, "bin"),
        String("ptxas") + String(Process::getExecutableSuffix()));
    if (!File::exists(ptxasPath))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(cudaRoot, set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outputArtifact);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<IOSFileArtifactRepresentation> ptxFile;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->requireFile(ArtifactKeep::No, ptxFile.writeRef())));
    SLANG_CHECK_ABORT(ptxFile != nullptr);

    TempPtxasOutput tempOutput;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-nvvm-ptxas"), tempOutput.lockPath)));
    tempOutput.cubinPath = tempOutput.lockPath + ".cubin";

    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, ptxasPath));
    commandLine.addArg("-arch=sm_75");
    commandLine.addArg("-v");
    commandLine.addArg(ptxFile->getPath());
    commandLine.addArg("-o");
    commandLine.addArg(tempOutput.cubinPath);

    ExecuteResult executeResult;
    SlangResult executeCallResult = ProcessUtil::execute(commandLine, executeResult);
    if (SLANG_FAILED(executeCallResult) || executeResult.resultCode != 0)
    {
        StringBuilder message;
        message << "ptxas stdout:\n" << executeResult.standardOutput;
        message << "\nptxas stderr:\n" << executeResult.standardError;
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeCallResult));
    SLANG_CHECK(executeResult.resultCode == 0);
    SLANG_CHECK(File::exists(tempOutput.cubinPath));
}
