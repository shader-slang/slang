// unit-test-separate-debug-without-spirv-opt.cpp

#include "core/slang-memory-file-system.h"
#include "core/slang-string.h"
#include "scoped-env-var.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;
using SlangUnitTest::ScopedEnvVar;

// Separate debug info (`-separate-debug-info`) splits a SPIR-V module into a stripped module and a
// full debug module. Slang does the split itself with a word-level filter, so the split must not
// depend on the downstream optimizer from `slang-glslang` (issue #13795):
//
// - At `-O0` the optimizer has no passes to run, so a split compile must succeed without
//   `slang-glslang` and must still produce both modules.
// - When `slang-glslang` is loaded for another reason (validation, or linking a precompiled
//   module), a `-O0` split compile must not call the optimizer, and splits the module that the
//   other step produced.
// - When the optimizer is required but missing (`-O1`), separate debug info must not change the
//   outcome of the compile: E00100 is reported, and the debug module is never left missing.
//
// These tests install a fake `slang-glslang` with `setSharedLibraryLoader` and compile through
// the API. Unlike the CLI test `tests/spirv/separate-debug-without-spirv-opt.slang`, the fake
// records which entry points ran (optimizer, validator, linker), the API returns both modules for
// inspection, and a crash fails the test rather than producing the same result code as a clean
// failure.

namespace
{

// Set by the fake library's entry points, so a test that serves the fake library can tell "this
// step ran" from "this step was skipped". They are never set in tests that refuse the library.
bool gFakeOptimizerWasCalled = false;
bool gFakeValidatorWasCalled = false;
bool gFakeLinkerWasCalled = false;

// Copy the input SPIR-V to the output unchanged, which is a valid answer to the optimize request.
int fakeCompileIdentity(glslang_CompileRequest_1_3* request)
{
    gFakeOptimizerWasCalled = true;
    if (!request || !request->outputFunc)
    {
        return 1;
    }
    const char* begin = (const char*)request->inputBegin;
    const char* end = (const char*)request->inputEnd;
    request->outputFunc(begin, size_t(end - begin), request->outputUserData);
    return 0;
}

bool fakeValidateReturningTrue(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeValidatorWasCalled = true;
    return true;
}

// The link result: the first input module (the importer's, which carries the build identifier)
// followed by an `OpSourceExtension "<kFakeLinkMarker>"` instruction. `OpSourceExtension` is a
// debug instruction, so a split of the link result keeps the marker only in the debug module,
// and a split of the native module never contains it. That is how the linked-split test tells
// which module was split. The buffer must outlive the `link` call, because the caller copies it
// only after `link` returns.
const char kFakeLinkMarker[] = "fake-link-output";
List<uint32_t> gFakeLinkResult;

int fakeLinkAppendingMarker(glslang_LinkRequest* request)
{
    gFakeLinkerWasCalled = true;
    if (!request || !request->modules || request->moduleCount < 1)
    {
        return false;
    }
    gFakeLinkResult.clear();
    gFakeLinkResult.addRange(request->modules[0], Index(request->moduleSizes[0]));

    // A SPIR-V literal string is NUL-terminated and padded with zeros to a word boundary.
    const uint32_t markerWordCount = uint32_t((sizeof(kFakeLinkMarker) + 3) / 4);
    const uint32_t kSpvOpSourceExtension = 4;
    gFakeLinkResult.add(((1 + markerWordCount) << 16) | kSpvOpSourceExtension);
    const Index markerStart = gFakeLinkResult.getCount();
    for (uint32_t i = 0; i < markerWordCount; ++i)
    {
        gFakeLinkResult.add(0);
    }
    memcpy(gFakeLinkResult.getBuffer() + markerStart, kFakeLinkMarker, sizeof(kFakeLinkMarker));

    request->linkResult = gFakeLinkResult.getBuffer();
    request->linkResultSize = size_t(gFakeLinkResult.getCount());
    return true;
}

// A `slang-glslang` that exists only as a symbol table with a working optimizer, validator and
// linker.
class FakeGlslangLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);
        if (symbol == "glslang_compile_1_3")
        {
            return (void*)fakeCompileIdentity;
        }
        if (symbol == "glslang_validateSPIRV")
        {
            return (void*)fakeValidateReturningTrue;
        }
        if (symbol == "glslang_linkSPIRV")
        {
            return (void*)fakeLinkAppendingMarker;
        }
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

// Whether the fake loader refuses every `slang-glslang` request, which leaves Slang with no
// optimizer, validator or linker, or answers it with the fake library.
enum class GlslangLibrary
{
    Missing,
    Faked,
};

class FakeLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeLoader(GlslangLibrary library)
        : m_library(library)
    {
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        // On unix the locator probes pthread before `slang-glslang`; refusing everything else
        // keeps the fake bound to the one library these tests are about.
        UnownedStringSlice request(path);
        if (request.indexOf(UnownedStringSlice("slang-glslang")) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        if (m_library == GlslangLibrary::Missing)
        {
            return SLANG_E_NOT_FOUND;
        }

        ComPtr<ISlangSharedLibrary> fakeLibrary(new FakeGlslangLibrary());
        *outLibrary = fakeLibrary.detach();
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

    GlslangLibrary m_library;
};

// The variable name appears in the debug module (as `OpName` and in the embedded source) but not
// in the stripped one, which tells the two modules apart.
const char kShaderSource[] = R"SLANG(
    RWStructuredBuffer<uint> gSeparateDebugOutput;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        gSeparateDebugOutput[tid.x] = tid.x + 1;
    }
)SLANG";

// The same shader with its arithmetic in an imported module. Precompiling that module for SPIR-V
// gives the program a second SPIR-V module, which is what makes `slang-emit.cpp` link.
const char kLibrarySource[] = R"SLANG(
    module separate_debug_lib;
    public uint addOne(uint x)
    {
        return x + 1;
    }
)SLANG";

const char kImportingShaderSource[] = R"SLANG(
    import separate_debug_lib;

    RWStructuredBuffer<uint> gSeparateDebugOutput;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        gSeparateDebugOutput[tid.x] = addOne(tid.x);
    }
)SLANG";

struct CompileOptions
{
    GlslangLibrary library = GlslangLibrary::Missing;
    SlangOptimizationLevel optimizationLevel = SLANG_OPTIMIZATION_LEVEL_NONE;
    bool separateDebugInfo = true;
    // Skipping validation keeps CI's `SLANG_RUN_SPIRV_VALIDATION=1` from loading the library.
    bool skipValidation = true;
    // Compile `kImportingShaderSource` against a precompiled `kLibrarySource`, so that a link runs.
    bool linkPrecompiledLibrary = false;
};

// What a compile produced. `code` is item 0 (the stripped module when separate debug info is on)
// and `debugCode` is item 1.
struct CompileOutcome
{
    SlangResult result = SLANG_FAIL;
    uint32_t itemCount = 0;
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> debugCode;
    String debugBuildIdentifier;
    String diagnostics;
};

void addIntOption(
    List<slang::CompilerOptionEntry>& ioEntries,
    slang::CompilerOptionName name,
    int value)
{
    slang::CompilerOptionEntry entry = {};
    entry.name = name;
    entry.value.kind = slang::CompilerOptionValueKind::Int;
    entry.value.intValue0 = value;
    ioEntries.add(entry);
}

// Compile the test shader to SPIR-V in a fresh global session that has the fake loader installed,
// and return what `getEntryPointCompileResult` produced. Front-end, precompile and link failures
// abort the test, because only code generation is under test. `outcome.diagnostics` holds only the
// code-generation diagnostics. The `gFake*WasCalled` flags cover the whole call, precompilation
// included.
CompileOutcome compileToSpirv(const CompileOptions& options)
{
    gFakeOptimizerWasCalled = false;
    gFakeValidatorWasCalled = false;
    gFakeLinkerWasCalled = false;

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(options.library));
    globalSession->setSharedLibraryLoader(loader);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    List<slang::CompilerOptionEntry> compilerOptions;
    addIntOption(
        compilerOptions,
        slang::CompilerOptionName::DebugInformation,
        SLANG_DEBUG_INFO_LEVEL_STANDARD);
    addIntOption(
        compilerOptions,
        slang::CompilerOptionName::Optimization,
        options.optimizationLevel);
    if (options.separateDebugInfo)
    {
        addIntOption(compilerOptions, slang::CompilerOptionName::EmitSeparateDebug, 1);
    }
    if (options.skipValidation)
    {
        addIntOption(compilerOptions, slang::CompilerOptionName::SkipSPIRVValidation, 1);
    }

    ComPtr<ISlangFileSystemExt> fileSystem(new MemoryFileSystem());
    auto& memoryFileSystem = *static_cast<MemoryFileSystem*>(fileSystem.get());
    memoryFileSystem.saveFile("separate_debug_lib.slang", kLibrarySource, strlen(kLibrarySource));

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = compilerOptions.getBuffer();
    sessionDesc.compilerOptionEntryCount = uint32_t(compilerOptions.getCount());
    sessionDesc.fileSystem = fileSystem;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    List<slang::IComponentType*> components;
    if (options.linkPrecompiledLibrary)
    {
        slang::IModule* library = session->loadModule("separate_debug_lib", diagnostics.writeRef());
        SLANG_CHECK_ABORT(library != nullptr);

        ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
        SLANG_CHECK_ABORT(
            library->queryInterface(
                slang::IModulePrecompileService_Experimental::getTypeGuid(),
                (void**)precompileService.writeRef()) == SLANG_OK);
        SLANG_CHECK_ABORT(
            precompileService->precompileForTarget(SLANG_SPIRV, diagnostics.writeRef()) ==
            SLANG_OK);
        components.add(library);
    }

    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        options.linkPrecompiledLibrary ? kImportingShaderSource : kShaderSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    components.add(module);
    components.add(entryPoint.get());
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components.getBuffer(),
            components.getCount(),
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType2> linkedProgram2;
    SLANG_CHECK_ABORT(
        linkedProgram->queryInterface(SLANG_IID_PPV_ARGS(linkedProgram2.writeRef())) == SLANG_OK);

    CompileOutcome outcome;
    ComPtr<slang::ICompileResult> compileResult;
    diagnostics.setNull();
    outcome.result = linkedProgram2->getEntryPointCompileResult(
        0,
        0,
        compileResult.writeRef(),
        diagnostics.writeRef());

    if (diagnostics && diagnostics->getBufferSize())
    {
        outcome.diagnostics = String(
            (const char*)diagnostics->getBufferPointer(),
            (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
    }

    if (SLANG_SUCCEEDED(outcome.result) && compileResult)
    {
        outcome.itemCount = compileResult->getItemCount();
        compileResult->getItemData(0, outcome.code.writeRef());
        if (outcome.itemCount > 1)
        {
            compileResult->getItemData(1, outcome.debugCode.writeRef());
        }
        ComPtr<slang::IMetadata> metadata;
        if (SLANG_SUCCEEDED(compileResult->getMetadata(metadata.writeRef())) && metadata)
        {
            outcome.debugBuildIdentifier = metadata->getDebugBuildIdentifier();
        }
    }
    return outcome;
}

bool blobContains(slang::IBlob* blob, const char* text)
{
    if (!blob)
    {
        return false;
    }
    const char* begin = (const char*)blob->getBufferPointer();
    UnownedStringSlice bytes(begin, begin + blob->getBufferSize());
    return bytes.indexOf(UnownedStringSlice(text)) >= 0;
}

bool diagnosticsContain(const CompileOutcome& outcome, const char* text)
{
    return outcome.diagnostics.getUnownedSlice().indexOf(UnownedStringSlice(text)) >= 0;
}

// Check that `outcome` holds a stripped module and a full debug module that share one build
// identifier, which is what `-separate-debug-info` promises.
void checkSplitModules(const CompileOutcome& outcome)
{
    SLANG_CHECK_ABORT(outcome.itemCount == 2);
    SLANG_CHECK_ABORT(outcome.code && outcome.debugCode);
    SLANG_CHECK(outcome.code->getBufferSize() < outcome.debugCode->getBufferSize());

    SLANG_CHECK(outcome.debugBuildIdentifier.getLength() != 0);
    SLANG_CHECK(blobContains(outcome.code, outcome.debugBuildIdentifier.getBuffer()));
    SLANG_CHECK(blobContains(outcome.debugCode, outcome.debugBuildIdentifier.getBuffer()));

    SLANG_CHECK(!blobContains(outcome.code, "gSeparateDebugOutput"));
    SLANG_CHECK(blobContains(outcome.debugCode, "gSeparateDebugOutput"));
}

} // namespace

// At `-O0` nothing needs `slang-glslang`, so a split compile must succeed with the library missing.
// A required load reports E00100, so its absence shows that the optimizer was not required. The
// library is still requested by the build-identifier hash, which reads the downstream version
// on a best-effort basis that cannot fail the compile.
SLANG_UNIT_TEST(separateDebugAtO0SucceedsWithoutSpirvOpt)
{
    const CompileOutcome outcome = compileToSpirv({.library = GlslangLibrary::Missing});

    SLANG_CHECK(!diagnosticsContain(outcome, "E00100"));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(outcome.result));
    checkSplitModules(outcome);
}

// Validation loads `slang-glslang`, but at `-O0` the optimizer has nothing to do, so a split
// compile must validate, must not call the optimizer, and must still produce both modules.
SLANG_UNIT_TEST(separateDebugAtO0WithValidationDoesNotCallOptimizer)
{
    ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");
    const CompileOutcome outcome =
        compileToSpirv({.library = GlslangLibrary::Faked, .skipValidation = false});

    SLANG_CHECK(gFakeValidatorWasCalled);
    SLANG_CHECK(!gFakeOptimizerWasCalled);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(outcome.result));
    checkSplitModules(outcome);
}

// Linking a precompiled module loads `slang-glslang`, but at `-O0` the optimizer has nothing to
// do, so a split compile must split the linked module without calling the optimizer. The fake
// link result carries a debug-only marker, which must end up in the debug module only.
SLANG_UNIT_TEST(separateDebugAtO0SplitsLinkedModule)
{
    const CompileOutcome outcome =
        compileToSpirv({.library = GlslangLibrary::Faked, .linkPrecompiledLibrary = true});

    SLANG_CHECK(gFakeLinkerWasCalled);
    SLANG_CHECK(!gFakeOptimizerWasCalled);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(outcome.result));
    checkSplitModules(outcome);
    SLANG_CHECK(blobContains(outcome.debugCode, kFakeLinkMarker));
    SLANG_CHECK(!blobContains(outcome.code, kFakeLinkMarker));
}

// At `-O1` the optimizer is required. With `slang-glslang` missing, the API compile reports E00100
// and still returns the natively emitted module, as it does without separate debug info; with
// separate debug info it returns both modules instead of crashing on a missing debug module.
SLANG_UNIT_TEST(separateDebugWithMissingSpirvOptMatchesNonSplitCompile)
{
    const CompileOutcome plain = compileToSpirv(
        {.library = GlslangLibrary::Missing,
         .optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT,
         .separateDebugInfo = false});
    SLANG_CHECK(diagnosticsContain(plain, "E00100"));
    SLANG_CHECK(SLANG_SUCCEEDED(plain.result));
    SLANG_CHECK(plain.itemCount == 1);

    const CompileOutcome split = compileToSpirv(
        {.library = GlslangLibrary::Missing,
         .optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT});
    SLANG_CHECK(diagnosticsContain(split, "E00100"));
    SLANG_CHECK(!diagnosticsContain(split, "E99997"));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(split.result));
    checkSplitModules(split);
}
