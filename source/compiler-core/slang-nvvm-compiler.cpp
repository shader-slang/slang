// slang-nvvm-compiler.cpp
#include "slang-nvvm-compiler.h"

#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "core/slang-shared-library.h"
#include "core/slang-string-slice-pool.h"
#include "core/slang-string-util.h"
#include "slang-artifact-associated-impl.h"
#include "slang-artifact-desc-util.h"
#include "slang-artifact-util.h"
#include "slang-com-helper.h"

namespace nvvm
{

typedef enum
{
    NVVM_SUCCESS = 0,
    NVVM_ERROR_OUT_OF_MEMORY = 1,
    NVVM_ERROR_PROGRAM_CREATION_FAILURE = 2,
    NVVM_ERROR_IR_VERSION_MISMATCH = 3,
    NVVM_ERROR_INVALID_INPUT = 4,
    NVVM_ERROR_INVALID_PROGRAM = 5,
    NVVM_ERROR_INVALID_IR = 6,
    NVVM_ERROR_INVALID_OPTION = 7,
    NVVM_ERROR_NO_MODULE_IN_PROGRAM = 8,
    NVVM_ERROR_COMPILATION = 9,
    NVVM_ERROR_CANCELLED = 10,
} nvvmResult;

typedef struct _nvvmProgram* nvvmProgram;

// Keep these declarations synchronized with NVIDIA's public libNVVM C API. Declaring the small
// dynamic ABI locally keeps CUDA headers and import libraries optional build dependencies.
// clang-format off
#define SLANG_NVVM_REQUIRED_FUNCS(x) \
    x(const char*, nvvmGetErrorString, (nvvmResult result)) \
    x(nvvmResult, nvvmVersion, (int* major, int* minor)) \
    x(nvvmResult, nvvmIRVersion, (int* majorIR, int* minorIR, int* majorDbg, int* minorDbg)) \
    x(nvvmResult, nvvmCreateProgram, (nvvmProgram* program)) \
    x(nvvmResult, nvvmDestroyProgram, (nvvmProgram* program)) \
    x(nvvmResult, nvvmAddModuleToProgram, (nvvmProgram program, const char* buffer, size_t size, const char* name)) \
    x(nvvmResult, nvvmVerifyProgram, (nvvmProgram program, int optionCount, const char** options)) \
    x(nvvmResult, nvvmCompileProgram, (nvvmProgram program, int optionCount, const char** options)) \
    x(nvvmResult, nvvmGetCompiledResultSize, (nvvmProgram program, size_t* size)) \
    x(nvvmResult, nvvmGetCompiledResult, (nvvmProgram program, char* result)) \
    x(nvvmResult, nvvmGetProgramLogSize, (nvvmProgram program, size_t* size)) \
    x(nvvmResult, nvvmGetProgramLog, (nvvmProgram program, char* log))

#define SLANG_NVVM_OPTIONAL_FUNCS(x) \
    x(nvvmResult, nvvmLazyAddModuleToProgram, (nvvmProgram program, const char* buffer, size_t size, const char* name)) \
    x(nvvmResult, nvvmLLVMVersion, (const char* arch, int* major))
// clang-format on

} // namespace nvvm

namespace Slang
{
using namespace nvvm;

static SlangResult _asSlangResult(nvvmResult result)
{
    switch (result)
    {
    case NVVM_SUCCESS:
        return SLANG_OK;
    case NVVM_ERROR_OUT_OF_MEMORY:
        return SLANG_E_OUT_OF_MEMORY;
    case NVVM_ERROR_INVALID_INPUT:
    case NVVM_ERROR_INVALID_OPTION:
        return SLANG_E_INVALID_ARG;
    case NVVM_ERROR_INVALID_PROGRAM:
        return SLANG_E_INTERNAL_FAIL;
    case NVVM_ERROR_CANCELLED:
        return SLANG_E_ABORT;
    case NVVM_ERROR_PROGRAM_CREATION_FAILURE:
    case NVVM_ERROR_IR_VERSION_MISMATCH:
    case NVVM_ERROR_INVALID_IR:
    case NVVM_ERROR_NO_MODULE_IN_PROGRAM:
    case NVVM_ERROR_COMPILATION:
    default:
        return SLANG_FAIL;
    }
}

static bool _isCompileRejection(nvvmResult result)
{
    switch (result)
    {
    case NVVM_ERROR_IR_VERSION_MISMATCH:
    case NVVM_ERROR_INVALID_IR:
    case NVVM_ERROR_INVALID_OPTION:
    case NVVM_ERROR_COMPILATION:
        return true;
    default:
        return false;
    }
}

static SlangResult _returnArtifact(
    SlangResult result,
    ComPtr<IArtifact>& artifact,
    IArtifact** outArtifact)
{
    // The code-generation caller extracts associated diagnostics before it checks `result`, so it
    // requires an artifact on every error path after the compiler has accepted the call.
    *outArtifact = artifact.detach();
    return result;
}

static void _appendRawDiagnostics(IArtifactDiagnostics* diagnostics, const UnownedStringSlice& text)
{
    if (!text.getLength())
        return;

    diagnostics->appendRaw(asCharSlice(text));
}

static void _setPlainFailure(
    IArtifactDiagnostics* diagnostics,
    SlangResult result,
    const UnownedStringSlice& text)
{
    diagnostics->setResult(result);
    _appendRawDiagnostics(diagnostics, text);
    diagnostics->requireErrorDiagnostic();
}

class NVVMDownstreamCompiler : public DownstreamCompilerBase
{
public:
    typedef DownstreamCompilerBase Super;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return false; }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getVersionString(slang::IBlob** outVersionString)
        SLANG_OVERRIDE;

    SlangResult init(ISlangSharedLibrary* library);

private:
    struct ScopeProgram
    {
        ScopeProgram(NVVMDownstreamCompiler* compiler, nvvmProgram program)
            : compiler(compiler), program(program)
        {
        }

        ~ScopeProgram()
        {
            if (program)
            {
                compiler->m_nvvmDestroyProgram(&program);
            }
        }

        NVVMDownstreamCompiler* compiler;
        nvvmProgram program;
    };

    SlangResult _calcCompileOptions(
        const CompileOptions& options,
        CommandLine& outCommandLine,
        String& outError);
    SlangResult _getProgramLog(nvvmProgram program, String& outLog);
    void _setNVVMFailure(
        IArtifactDiagnostics* diagnostics,
        const char* operation,
        nvvmResult result,
        const String& log);

#define SLANG_NVVM_MEMBER_FUNC(ret, name, params) ret(*m_##name) params = nullptr;
    SLANG_NVVM_REQUIRED_FUNCS(SLANG_NVVM_MEMBER_FUNC)
    SLANG_NVVM_OPTIONAL_FUNCS(SLANG_NVVM_MEMBER_FUNC)
#undef SLANG_NVVM_MEMBER_FUNC

    ComPtr<ISlangSharedLibrary> m_sharedLibrary;
    String m_libraryPath;
    String m_toolkitRoot;
    int m_irMajor = 0;
    int m_irMinor = 0;
    int m_debugMetadataMajor = 0;
    int m_debugMetadataMinor = 0;
};

SlangResult NVVMDownstreamCompiler::init(ISlangSharedLibrary* library)
{
    if (!library)
        return SLANG_E_INVALID_ARG;

#define SLANG_NVVM_GET_REQUIRED_FUNC(ret, name, params)       \
    m_##name = (ret(*) params)library->findFuncByName(#name); \
    if (!m_##name)                                            \
        return SLANG_FAIL;
    SLANG_NVVM_REQUIRED_FUNCS(SLANG_NVVM_GET_REQUIRED_FUNC)
#undef SLANG_NVVM_GET_REQUIRED_FUNC

#define SLANG_NVVM_GET_OPTIONAL_FUNC(ret, name, params) \
    m_##name = (ret(*) params)library->findFuncByName(#name);
    SLANG_NVVM_OPTIONAL_FUNCS(SLANG_NVVM_GET_OPTIONAL_FUNC)
#undef SLANG_NVVM_GET_OPTIONAL_FUNC

    int major = 0;
    int minor = 0;
    SLANG_RETURN_ON_FAIL(_asSlangResult(m_nvvmVersion(&major, &minor)));
    SLANG_RETURN_ON_FAIL(_asSlangResult(
        m_nvvmIRVersion(&m_irMajor, &m_irMinor, &m_debugMetadataMajor, &m_debugMetadataMinor)));

    m_sharedLibrary = library;
    m_desc.type = SLANG_PASS_THROUGH_NVVM;
    m_desc.version.set(major, minor);

    m_libraryPath = SharedLibraryUtils::getSharedLibraryFileName((void*)m_nvvmCreateProgram);
    if (m_libraryPath.getLength())
    {
        String libraryDirectory = Path::getParentDirectory(m_libraryPath);
        if (Path::getFileName(libraryDirectory)
                .getUnownedSlice()
                .caseInsensitiveEquals(toSlice("x64")))
        {
            libraryDirectory = Path::getParentDirectory(libraryDirectory);
        }

        const String binaryDirectoryName = Path::getFileName(libraryDirectory);
        const bool isNVVMBinaryDirectory =
            binaryDirectoryName.getUnownedSlice().caseInsensitiveEquals(toSlice("bin")) ||
            binaryDirectoryName.getUnownedSlice().caseInsensitiveEquals(toSlice("lib")) ||
            binaryDirectoryName.getUnownedSlice().caseInsensitiveEquals(toSlice("lib64"));
        const String nvvmRoot = Path::getParentDirectory(libraryDirectory);
        if (isNVVMBinaryDirectory &&
            Path::getFileName(nvvmRoot).getUnownedSlice().caseInsensitiveEquals(toSlice("nvvm")))
        {
            m_toolkitRoot = Path::getParentDirectory(nvvmRoot);
        }
    }

    return SLANG_OK;
}

SlangResult NVVMDownstreamCompiler::getVersionString(slang::IBlob** outVersionString)
{
    if (!outVersionString)
        return SLANG_E_INVALID_ARG;

    StringBuilder version;
    m_desc.version.append(version);
    version << " nvvm-ir=" << m_irMajor << "." << m_irMinor;
    version << " debug=" << m_debugMetadataMajor << "." << m_debugMetadataMinor;
    version << " library="
            << SharedLibraryUtils::getSharedLibraryTimestamp((void*)m_nvvmCreateProgram);

    *outVersionString = StringBlob::moveCreate(version).detach();
    return SLANG_OK;
}

SlangResult NVVMDownstreamCompiler::_calcCompileOptions(
    const CompileOptions& options,
    CommandLine& outCommandLine,
    String& outError)
{
    bool hasArchitecture = false;
    SemanticVersion architecture;
    for (const auto& requirement : options.requiredCapabilityVersions)
    {
        if (requirement.kind == CompileOptions::CapabilityVersion::Kind::CUDASM &&
            (!hasArchitecture || requirement.version > architecture))
        {
            architecture = requirement.version;
            hasArchitecture = true;
        }
    }

    if (!hasArchitecture)
    {
        outError = "libNVVM requires an explicit CUDA compute architecture";
        return SLANG_E_INVALID_ARG;
    }
    if (architecture.m_major <= 0 || architecture.m_minor < 0 || architecture.m_minor > 9)
    {
        outError = "libNVVM received an invalid CUDA compute architecture";
        return SLANG_E_INVALID_ARG;
    }

    StringBuilder architectureOption;
    architectureOption << "-arch=compute_" << architecture.m_major;
    architectureOption << char('0' + architecture.m_minor);
    outCommandLine.addArg(architectureOption);

    if (options.debugInfoType == DebugInfoType::Maximal)
    {
        if (options.optimizationLevel != OptimizationLevel::None)
        {
            outError = "libNVVM maximal debug information requires optimization to be disabled";
            return SLANG_E_INVALID_ARG;
        }
        outCommandLine.addArg("-g");
        outCommandLine.addArg("-opt=0");
    }
    else
    {
        switch (options.optimizationLevel)
        {
        case OptimizationLevel::None:
            outCommandLine.addArg("-opt=0");
            break;
        case OptimizationLevel::Default:
        case OptimizationLevel::High:
        case OptimizationLevel::Maximal:
            outCommandLine.addArg("-opt=3");
            break;
        }
    }

    switch (options.denormalModeFp32)
    {
    case CompileOptions::FloatingPointDenormalMode::Any:
        break;
    case CompileOptions::FloatingPointDenormalMode::Preserve:
        outCommandLine.addArg("-ftz=0");
        break;
    case CompileOptions::FloatingPointDenormalMode::FlushToZero:
        outCommandLine.addArg("-ftz=1");
        break;
    }

    switch (options.floatingPointMode)
    {
    case FloatingPointMode::Default:
        break;
    case FloatingPointMode::Precise:
        outCommandLine.addArg("-prec-div=1");
        outCommandLine.addArg("-prec-sqrt=1");
        outCommandLine.addArg("-fma=0");
        break;
    case FloatingPointMode::Fast:
        outCommandLine.addArg("-prec-div=0");
        outCommandLine.addArg("-prec-sqrt=0");
        outCommandLine.addArg("-fma=1");
        break;
    }

    for (const auto& argument : options.compilerSpecificArguments)
    {
        outCommandLine.addArg(asString(argument));
    }

    return SLANG_OK;
}

SlangResult NVVMDownstreamCompiler::_getProgramLog(nvvmProgram program, String& outLog)
{
    outLog = String();

    size_t logSize = 0;
    nvvmResult result = m_nvvmGetProgramLogSize(program, &logSize);
    if (result != NVVM_SUCCESS)
        return _asSlangResult(result);
    if (logSize == 0)
        return SLANG_OK;

    List<char> buffer;
    buffer.setCount(Index(logSize));
    result = m_nvvmGetProgramLog(program, buffer.getBuffer());
    if (result != NVVM_SUCCESS)
        return _asSlangResult(result);

    if (logSize > 0 && buffer[Index(logSize - 1)] == 0)
        --logSize;

    StringBuilder builder;
    builder.append(buffer.getBuffer(), Index(logSize));
    outLog = builder.produceString();
    return SLANG_OK;
}

void NVVMDownstreamCompiler::_setNVVMFailure(
    IArtifactDiagnostics* diagnostics,
    const char* operation,
    nvvmResult result,
    const String& log)
{
    StringBuilder message;
    message << "libNVVM " << operation << " failed";
    if (log.getLength())
    {
        message << ":\n" << log;
    }
    else
    {
        const char* error = m_nvvmGetErrorString(result);
        if (error && error[0])
            message << ": " << error;
    }

    diagnostics->setResult(_asSlangResult(result));
    _appendRawDiagnostics(diagnostics, message.getUnownedSlice());
    diagnostics->requireErrorDiagnostic();
}

SlangResult NVVMDownstreamCompiler::compile(
    const DownstreamCompileOptions& inOptions,
    IArtifact** outArtifact)
{
    if (!outArtifact)
        return SLANG_E_INVALID_ARG;
    *outArtifact = nullptr;

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_PTX);
    auto diagnostics = ArtifactDiagnostics::create();
    ArtifactUtil::addAssociated(artifact, diagnostics);

    if (!isVersionCompatible(inOptions))
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_NOT_IMPLEMENTED,
            toSlice("Incompatible libNVVM downstream compile options"));
        return _returnArtifact(SLANG_E_NOT_IMPLEMENTED, artifact, outArtifact);
    }

    CompileOptions options = getCompatibleVersion(&inOptions);
    if (options.targetType != SLANG_PTX)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INVALID_ARG,
            toSlice("libNVVM can only produce PTX artifacts"));
        return _returnArtifact(SLANG_E_INVALID_ARG, artifact, outArtifact);
    }
    if (options.sourceArtifacts.count != 1)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INVALID_ARG,
            toSlice("libNVVM requires exactly one NVVM IR source artifact"));
        return _returnArtifact(SLANG_E_INVALID_ARG, artifact, outArtifact);
    }
    if (options.libraries.count != 0)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_NOT_IMPLEMENTED,
            toSlice("libNVVM library inputs are not supported by this backend slice"));
        return _returnArtifact(SLANG_E_NOT_IMPLEMENTED, artifact, outArtifact);
    }

    IArtifact* sourceArtifact = options.sourceArtifacts[0];
    const ArtifactDesc expectedAssemblySourceDesc = ArtifactDesc::make(
        ArtifactKind::Assembly,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel,
        0);
    const ArtifactDesc expectedBitcodeSourceDesc = ArtifactDesc::make(
        ArtifactKind::ObjectCode,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel,
        0);
    if (!sourceArtifact || (sourceArtifact->getDesc() != expectedAssemblySourceDesc &&
                            sourceArtifact->getDesc() != expectedBitcodeSourceDesc))
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INVALID_ARG,
            toSlice(
                "libNVVM requires an Assembly + LLVMIR + Kernel or ObjectCode + LLVMIR + Kernel "
                "source artifact"));
        return _returnArtifact(SLANG_E_INVALID_ARG, artifact, outArtifact);
    }

    CommandLine commandLine;
    String optionError;
    SlangResult optionResult = _calcCompileOptions(options, commandLine, optionError);
    if (SLANG_FAILED(optionResult))
    {
        _setPlainFailure(diagnostics, optionResult, optionError.getUnownedSlice());
        return _returnArtifact(optionResult, artifact, outArtifact);
    }

    ComPtr<ISlangBlob> sourceBlob;
    SlangResult sourceResult = sourceArtifact->loadBlob(ArtifactKeep::Yes, sourceBlob.writeRef());
    if (SLANG_FAILED(sourceResult))
    {
        _setPlainFailure(
            diagnostics,
            sourceResult,
            toSlice("Unable to load the NVVM IR artifact as an in-memory blob"));
        return _returnArtifact(sourceResult, artifact, outArtifact);
    }

    nvvmProgram program = nullptr;
    nvvmResult nvvmResultCode = m_nvvmCreateProgram(&program);
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        _setNVVMFailure(diagnostics, "program creation", nvvmResultCode, String());
        return _returnArtifact(_asSlangResult(nvvmResultCode), artifact, outArtifact);
    }
    if (!program)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INTERNAL_FAIL,
            toSlice("libNVVM reported successful program creation without returning a program"));
        return _returnArtifact(SLANG_E_INTERNAL_FAIL, artifact, outArtifact);
    }
    ScopeProgram scopeProgram(this, program);

    nvvmResultCode = m_nvvmAddModuleToProgram(
        program,
        static_cast<const char*>(sourceBlob->getBufferPointer()),
        sourceBlob->getBufferSize(),
        "slang-nvvm-input");
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        String log;
        _getProgramLog(program, log);
        _setNVVMFailure(diagnostics, "module loading", nvvmResultCode, log);
        return _returnArtifact(_asSlangResult(nvvmResultCode), artifact, outArtifact);
    }

    List<const char*> optionPointers;
    optionPointers.setCount(commandLine.m_args.getCount());
    for (Index i = 0; i < commandLine.m_args.getCount(); ++i)
        optionPointers[i] = commandLine.m_args[i].getBuffer();

    nvvmResultCode =
        m_nvvmVerifyProgram(program, int(optionPointers.getCount()), optionPointers.getBuffer());
    String verifierLog;
    SlangResult verifierLogResult = _getProgramLog(program, verifierLog);
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        _setNVVMFailure(diagnostics, "verification", nvvmResultCode, verifierLog);
        const SlangResult result =
            _isCompileRejection(nvvmResultCode) ? SLANG_OK : _asSlangResult(nvvmResultCode);
        return _returnArtifact(result, artifact, outArtifact);
    }
    if (SLANG_FAILED(verifierLogResult))
    {
        _setPlainFailure(
            diagnostics,
            verifierLogResult,
            toSlice("Unable to retrieve the libNVVM verifier log"));
        return _returnArtifact(verifierLogResult, artifact, outArtifact);
    }
    if (verifierLog.getLength())
        _appendRawDiagnostics(diagnostics, verifierLog.getUnownedSlice());

    nvvmResultCode =
        m_nvvmCompileProgram(program, int(optionPointers.getCount()), optionPointers.getBuffer());
    String compilerLog;
    SlangResult compilerLogResult = _getProgramLog(program, compilerLog);
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        _setNVVMFailure(diagnostics, "compilation", nvvmResultCode, compilerLog);
        const SlangResult result =
            _isCompileRejection(nvvmResultCode) ? SLANG_OK : _asSlangResult(nvvmResultCode);
        return _returnArtifact(result, artifact, outArtifact);
    }
    if (SLANG_FAILED(compilerLogResult))
    {
        _setPlainFailure(
            diagnostics,
            compilerLogResult,
            toSlice("Unable to retrieve the libNVVM compiler log"));
        return _returnArtifact(compilerLogResult, artifact, outArtifact);
    }
    if (compilerLog.getLength())
        _appendRawDiagnostics(diagnostics, compilerLog.getUnownedSlice());

    size_t resultSize = 0;
    nvvmResultCode = m_nvvmGetCompiledResultSize(program, &resultSize);
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        _setNVVMFailure(diagnostics, "result-size query", nvvmResultCode, String());
        return _returnArtifact(_asSlangResult(nvvmResultCode), artifact, outArtifact);
    }
    if (resultSize == 0)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INTERNAL_FAIL,
            toSlice("libNVVM returned an empty compiled-result buffer"));
        return _returnArtifact(SLANG_E_INTERNAL_FAIL, artifact, outArtifact);
    }

    List<uint8_t> ptx;
    ptx.setCount(Index(resultSize));
    nvvmResultCode = m_nvvmGetCompiledResult(program, (char*)ptx.getBuffer());
    if (nvvmResultCode != NVVM_SUCCESS)
    {
        _setNVVMFailure(diagnostics, "result retrieval", nvvmResultCode, String());
        return _returnArtifact(_asSlangResult(nvvmResultCode), artifact, outArtifact);
    }

    // The API size includes its C-string terminator. Enforce that boundary contract before
    // removing the terminator so a truncated vendor result cannot silently become a valid-looking
    // artifact.
    if (!ptx.getCount() || ptx.getLast() != 0)
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INTERNAL_FAIL,
            toSlice("libNVVM returned compiled PTX without a trailing terminator"));
        return _returnArtifact(SLANG_E_INTERNAL_FAIL, artifact, outArtifact);
    }
    ptx.removeLast();
    if (!ptx.getCount())
    {
        _setPlainFailure(
            diagnostics,
            SLANG_E_INTERNAL_FAIL,
            toSlice("libNVVM returned an empty compiled PTX payload"));
        return _returnArtifact(SLANG_E_INTERNAL_FAIL, artifact, outArtifact);
    }

    diagnostics->setResult(SLANG_OK);
    artifact->addRepresentationUnknown(ListBlob::moveCreate(ptx));
    return _returnArtifact(SLANG_OK, artifact, outArtifact);
}

static String _getLibraryLoadPath(const String& decoratedPath)
{
    const UnownedStringSlice extension = Path::getPathExt(decoratedPath.getUnownedSlice());
#if SLANG_WINDOWS_FAMILY
    if (extension.caseInsensitiveEquals(toSlice("dll")))
        return Path::getPathWithoutExt(decoratedPath);
#elif SLANG_LINUX_FAMILY
    const String fileName = Path::getFileName(decoratedPath);
    if (fileName.getUnownedSlice().indexOf(toSlice(".so.")) >= 0)
        return decoratedPath;
    if (extension == "so")
    {
        String baseName = Path::getFileNameWithoutExt(fileName);
        if (baseName.getUnownedSlice().startsWith("lib"))
            baseName = baseName.subString(3, baseName.getLength() - 3);
        return Path::combine(Path::getParentDirectory(decoratedPath), baseName);
    }
#elif SLANG_APPLE_FAMILY
    if (extension == "dylib")
    {
        String baseName = Path::getFileNameWithoutExt(decoratedPath);
        if (baseName.getUnownedSlice().startsWith("lib"))
            baseName = baseName.subString(3, baseName.getLength() - 3);
        return Path::combine(Path::getParentDirectory(decoratedPath), baseName);
    }
#endif
    return decoratedPath;
}

struct NVVMLibraryPathVisitor : Path::Visitor
{
    struct Candidate
    {
        String loadPath;
        String fileName;
        List<Int> versionComponents;
    };

    static bool _isBefore(const Candidate& left, const Candidate& right)
    {
        const Index componentCount =
            left.versionComponents.getCount() > right.versionComponents.getCount()
                ? left.versionComponents.getCount()
                : right.versionComponents.getCount();
        for (Index i = 0; i < componentCount; ++i)
        {
            const Int leftComponent =
                i < left.versionComponents.getCount() ? left.versionComponents[i] : 0;
            const Int rightComponent =
                i < right.versionComponents.getCount() ? right.versionComponents[i] : 0;
            if (leftComponent != rightComponent)
                return leftComponent < rightComponent;
        }
        return compare(left.fileName.getUnownedSlice(), right.fileName.getUnownedSlice()) < 0;
    }

    static void _parseVersionComponents(
        const UnownedStringSlice& version,
        char separator,
        List<Int>& outComponents)
    {
        List<UnownedStringSlice> componentSlices;
        StringUtil::split(version, separator, componentSlices);
        for (const auto& componentSlice : componentSlices)
        {
            Int component = 0;
            if (SLANG_FAILED(StringUtil::parseInt(componentSlice, component)) || component < 0)
            {
                outComponents.clear();
                return;
            }
            outComponents.add(component);
        }
    }

    static void _getVersionComponents(const UnownedStringSlice& fileName, List<Int>& outComponents)
    {
#if SLANG_WINDOWS_FAMILY
        const UnownedStringSlice prefix = toSlice("nvvm64_");
        const UnownedStringSlice suffix = toSlice(".dll");
        if (fileName.startsWithCaseInsensitive(prefix) && fileName.endsWithCaseInsensitive(suffix))
        {
            const Index versionLength =
                fileName.getLength() - prefix.getLength() - suffix.getLength();
            _parseVersionComponents(
                fileName.subString(prefix.getLength(), versionLength),
                '_',
                outComponents);
        }
#elif SLANG_LINUX_FAMILY
        const UnownedStringSlice marker = toSlice(".so.");
        const Index markerIndex = fileName.indexOf(marker);
        if (markerIndex >= 0)
        {
            _parseVersionComponents(
                fileName.tail(markerIndex + marker.getLength()),
                '.',
                outComponents);
        }
#else
        SLANG_UNUSED(fileName);
        SLANG_UNUSED(outComponents);
#endif
    }

    static bool _isCandidate(const UnownedStringSlice& fileName)
    {
#if SLANG_WINDOWS_FAMILY
        if (fileName.caseInsensitiveEquals(toSlice("nvvm.dll")))
            return true;
        List<Int> versionComponents;
        _getVersionComponents(fileName, versionComponents);
        return versionComponents.getCount() != 0;
#elif SLANG_LINUX_FAMILY
        if (fileName == "libnvvm.so")
            return true;
        if (!fileName.startsWith(toSlice("libnvvm.so.")))
            return false;
        List<Int> versionComponents;
        _getVersionComponents(fileName, versionComponents);
        return versionComponents.getCount() != 0;
#elif SLANG_APPLE_FAMILY
        return fileName == "libnvvm.dylib";
#else
        SLANG_UNUSED(fileName);
        return false;
#endif
    }

    void accept(Path::Type type, const UnownedStringSlice& fileName) SLANG_OVERRIDE
    {
        if (type != Path::Type::File || !_isCandidate(fileName))
            return;

        Candidate candidate;
        candidate.fileName = fileName;
        candidate.loadPath = _getLibraryLoadPath(Path::combine(directory, fileName));
        _getVersionComponents(fileName, candidate.versionComponents);
        for (const auto& existing : candidates)
        {
            if (existing.loadPath == candidate.loadPath)
                return;
        }
        candidates.add(candidate);
    }

    void find(const String& path)
    {
        SlangPathType pathType;
        if (SLANG_FAILED(Path::getPathType(path, &pathType)) ||
            pathType != SLANG_PATH_TYPE_DIRECTORY)
            return;

        directory = path;
        Path::find(path, nullptr, this);
    }

    void sort() { candidates.sort(_isBefore); }

    String directory;
    List<Candidate> candidates;
};

static void _addUniqueDirectory(const String& path, List<String>& ioDirectories)
{
    if (path.getLength() && ioDirectories.indexOf(path) < 0)
        ioDirectories.add(path);
}

static void _addToolkitDirectories(const String& root, List<String>& ioDirectories)
{
    if (!root.getLength())
        return;

    _addUniqueDirectory(root, ioDirectories);
#if SLANG_WINDOWS_FAMILY
    const String nvvmBin = Path::combine(root, "nvvm", "bin");
    const String rootBin = Path::combine(root, "bin");
    _addUniqueDirectory(nvvmBin, ioDirectories);
    _addUniqueDirectory(Path::combine(nvvmBin, "x64"), ioDirectories);
    _addUniqueDirectory(rootBin, ioDirectories);
    _addUniqueDirectory(Path::combine(rootBin, "x64"), ioDirectories);
#else
    _addUniqueDirectory(Path::combine(root, "nvvm", "lib64"), ioDirectories);
    _addUniqueDirectory(Path::combine(root, "nvvm", "lib"), ioDirectories);
    _addUniqueDirectory(Path::combine(root, "lib64"), ioDirectories);
    _addUniqueDirectory(Path::combine(root, "lib"), ioDirectories);
#endif
}

static SlangResult _loadFromDirectories(
    const List<String>& directories,
    ISlangSharedLibraryLoader* loader,
    ComPtr<ISlangSharedLibrary>& outLibrary)
{
    bool sawCandidate = false;
    SlangResult firstLoadFailure = SLANG_E_NOT_FOUND;
    for (const auto& directory : directories)
    {
        NVVMLibraryPathVisitor visitor;
        visitor.find(directory);
        visitor.sort();

        // Directory order expresses discovery precedence; version ordering is only used to choose
        // among candidates within one directory.
        for (Index i = visitor.candidates.getCount() - 1; i >= 0; --i)
        {
            sawCandidate = true;
            const SlangResult loadResult = loader->loadSharedLibrary(
                visitor.candidates[i].loadPath.getBuffer(),
                outLibrary.writeRef());
            if (SLANG_SUCCEEDED(loadResult))
            {
                return SLANG_OK;
            }
            if (firstLoadFailure == SLANG_E_NOT_FOUND && loadResult != SLANG_E_NOT_FOUND)
                firstLoadFailure = loadResult;
        }
    }
    return sawCandidate ? (firstLoadFailure == SLANG_E_NOT_FOUND ? SLANG_FAIL : firstLoadFailure)
                        : SLANG_E_NOT_FOUND;
}

static bool _looksLikeCudaPath(const UnownedStringSlice& path)
{
    List<UnownedStringSlice> components;
    Path::split(path, components);
    for (const auto& component : components)
    {
        if (component.startsWithCaseInsensitive(toSlice("cuda")))
            return true;
    }
    return false;
}

static void _addEnvironmentToolkitDirectories(
    const UnownedStringSlice& variableName,
    List<String>& ioDirectories)
{
    StringBuilder path;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(variableName, path)) &&
        path.getLength())
    {
        _addToolkitDirectories(path, ioDirectories);
    }
}

static void _addAutomaticSearchDirectories(List<String>& outDirectories)
{
    StringBuilder instancePath;
    if (SLANG_SUCCEEDED(PlatformUtil::getInstancePath(instancePath)))
        _addUniqueDirectory(instancePath, outDirectories);

    _addEnvironmentToolkitDirectories(toSlice("LIBNVVM_HOME"), outDirectories);
    _addEnvironmentToolkitDirectories(toSlice("CUDA_PATH"), outDirectories);
    _addEnvironmentToolkitDirectories(toSlice("CUDA_HOME"), outDirectories);

    StringBuilder pathValue;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(toSlice("PATH"), pathValue)))
    {
        List<UnownedStringSlice> pathEntries;
#if SLANG_WINDOWS_FAMILY
        StringUtil::split(pathValue.getUnownedSlice(), ';', pathEntries);
#else
        StringUtil::split(pathValue.getUnownedSlice(), ':', pathEntries);
#endif
        StringSlicePool visited(StringSlicePool::Style::Empty);
        for (const auto& entry : pathEntries)
        {
            if (!entry.getLength() || visited.has(entry) || !_looksLikeCudaPath(entry))
                continue;
            visited.add(entry);

            const String entryPath(entry);
            _addUniqueDirectory(entryPath, outDirectories);
            String toolkitCandidate = Path::getParentDirectory(entryPath);
#if SLANG_WINDOWS_FAMILY
            const String entryName = Path::getFileName(entryPath);
            const String parentName = Path::getFileName(toolkitCandidate);
            if (entryName.getUnownedSlice().caseInsensitiveEquals(toSlice("x64")) &&
                parentName.getUnownedSlice().caseInsensitiveEquals(toSlice("bin")))
            {
                toolkitCandidate = Path::getParentDirectory(toolkitCandidate);
            }
#endif
            _addToolkitDirectories(toolkitCandidate, outDirectories);
        }
    }
}

static SlangResult _createCompiler(ISlangSharedLibrary* library, DownstreamCompilerSet* set)
{
    auto compiler = new NVVMDownstreamCompiler;
    ComPtr<IDownstreamCompiler> compilerInterface(compiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));
    set->addCompiler(compilerInterface);
    return SLANG_OK;
}

/* static */ SlangResult NVVMDownstreamCompilerUtil::locateCompilers(
    const String& path,
    ISlangSharedLibraryLoader* loader,
    DownstreamCompilerSet* set)
{
    if (!loader || !set)
        return SLANG_E_INVALID_ARG;

    ComPtr<ISlangSharedLibrary> library;
    if (path.getLength())
    {
        SlangPathType pathType;
        if (SLANG_SUCCEEDED(Path::getPathType(path, &pathType)) &&
            pathType == SLANG_PATH_TYPE_DIRECTORY)
        {
            List<String> directories;
            _addToolkitDirectories(path, directories);
            SLANG_RETURN_ON_FAIL(_loadFromDirectories(directories, loader, library));
        }
        else
        {
            const String loadPath = _getLibraryLoadPath(path);
            SLANG_RETURN_ON_FAIL(
                loader->loadSharedLibrary(loadPath.getBuffer(), library.writeRef()));
        }
        return _createCompiler(library, set);
    }

    // Try the logical name before filesystem probing. Besides supporting platform loader paths,
    // this is the stable dependency-injection seam used by fake-library tests.
    if (SLANG_SUCCEEDED(loader->loadSharedLibrary("nvvm", library.writeRef())))
        return _createCompiler(library, set);

    List<String> directories;
    _addAutomaticSearchDirectories(directories);
    SLANG_RETURN_ON_FAIL(_loadFromDirectories(directories, loader, library));
    return _createCompiler(library, set);
}

} // namespace Slang
