// unit-test-downstream-linking-unavailable.cpp

#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-glslang-compiler.h"
#include "core/slang-memory-file-system.h"
#include "core/slang-shared-library.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// A `slang-glslang` that loads but does not export `glslang_linkSPIRV` must report the missing
// capability rather than call through a null function pointer.
//
// `init` only rejects a library outright when every `glslang_compile*` entry point is missing, so a
// successfully-initialized compiler may still have a null `m_link`. `link` must therefore report
// the missing capability rather than call through the pointer.
//
// Reaching that call needs *two* SPIR-V modules: `slang-emit.cpp` only links when
// `spirvFiles.getCount() > 1`, which happens when an imported module carries an
// `EmbeddedDownstreamIR` blob from `precompileForTarget`. A single-module compile never gets there,
// which is why the tests below build a library module and precompile it before compiling the
// importer.
//
// The condition is environmental rather than source-level, so no `.slang` test can reach it.
// `setSharedLibraryLoader` installs a fake loader whose symbol table the test controls.

namespace
{

// Which linker the fake library exports, covering the three states the caller must tell apart.
enum class FakeLinkerState
{
    Absent,
    PresentRejecting,
    PresentAccepting,
};

// Set by the fake linkers so the tests can tell "returned failure" from "was never called".
bool gFakeLinkerWasCalled = false;

// Set when the fake library is handed out, and when the compile entry point runs. Without these a
// test that silently never loaded the fake library is indistinguishable from one that loaded it and
// found nothing to report.
bool gFakeLibraryWasLoaded = false;
bool gFakeCompileWasCalled = false;

int fakeLinkReturningFailure(glslang_LinkRequest* request)
{
    SLANG_UNUSED(request);
    gFakeLinkerWasCalled = true;
    return false;
}

// Return the first input module as the link result, which is a well-formed answer to the request:
// the caller only reads `linkResult`/`linkResultSize` and wraps them in a blob. Echoing an input
// keeps the fake honest -- the bytes handed back are bytes the compiler produced. The buffer is
// owned by the caller's `spirvFiles` entry, which outlives the `link` call.
int fakeLinkReturningFirstModule(glslang_LinkRequest* request)
{
    gFakeLinkerWasCalled = true;
    if (!request || !request->modules || request->moduleCount < 1)
    {
        return false;
    }
    request->linkResult = request->modules[0];
    request->linkResultSize = request->moduleSizes[0];
    return true;
}

// Accept whatever it is handed. These tests are about the linking path, so validation must not be
// what decides their outcome.
bool fakeValidateReturningTrue(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    return true;
}

// Copy the input SPIR-V straight to the output, a valid response to the
// `GLSLANG_ACTION_OPTIMIZE_SPIRV` request `slang-emit.cpp` issues after linking.
int fakeCompileIdentity(glslang_CompileRequest_1_3* request)
{
    gFakeCompileWasCalled = true;
    if (!request || !request->outputFunc)
    {
        return 1;
    }
    const char* begin = (const char*)request->inputBegin;
    const char* end = (const char*)request->inputEnd;
    request->outputFunc(begin, size_t(end - begin), request->outputUserData);
    return 0;
}

// A shared library that exists only as a symbol table. `linkerState` decides whether
// `glslang_linkSPIRV` resolves and what it answers, which is the whole point of the test.
class FakeGlslangLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeGlslangLibrary(FakeLinkerState linkerState)
        : m_linkerState(linkerState)
    {
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);

        // At least one compile entry point must resolve, otherwise `init` rejects the library and
        // we would be testing the load-failure path instead of the missing-capability one. Only the
        // _1_3 symbol is exported, because that is the signature `fakeCompileIdentity` has -- and
        // the one `_invoke` prefers.
        if (symbol == "glslang_compile_1_3")
        {
            return (void*)fakeCompileIdentity;
        }
        // A null `m_validate` makes validation fail, which is a compile error, so when validation
        // is enabled externally every compile against this fake dies before reaching the linking
        // behaviour under test.
        if (symbol == "glslang_validateSPIRV")
        {
            return (void*)fakeValidateReturningTrue;
        }
        if (symbol == "glslang_linkSPIRV")
        {
            switch (m_linkerState)
            {
            case FakeLinkerState::PresentRejecting:
                return (void*)fakeLinkReturningFailure;
            case FakeLinkerState::PresentAccepting:
                return (void*)fakeLinkReturningFirstModule;
            default:
                return nullptr;
            }
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

    FakeLinkerState m_linkerState;
};

class FakeLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeLoader(FakeLinkerState linkerState)
        : m_linkerState(linkerState)
    {
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        // Match the bare logical name: platform decoration and the version suffix are applied by
        // `DefaultSharedLibraryLoader` before reaching here. On unix
        // `locateGlslangSpirvDownstreamCompiler` probes pthread first, and failing those keeps the
        // fake library bound to the one request we care about.
        UnownedStringSlice request(path);
        if (request.indexOf(UnownedStringSlice("slang-glslang")) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        gFakeLibraryWasLoaded = true;
        ComPtr<ISlangSharedLibrary> library(new FakeGlslangLibrary(m_linkerState));
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

    FakeLinkerState m_linkerState;
};

// Locate the SPIRV-opt compiler the way `slang-check.cpp` does, but against our fake loader, and
// hand back the `link` implementation under test.
IDownstreamCompiler* getFakeSpirvOptCompiler(
    RefPtr<DownstreamCompilerSet>& ioSet,
    ISlangSharedLibraryLoader* loader)
{
    ioSet = new DownstreamCompilerSet;
    if (SLANG_FAILED(SpirvOptDownstreamCompilerUtil::locateCompilers(String(), loader, ioSet)))
    {
        return nullptr;
    }
    DownstreamCompilerDesc desc;
    desc.type = SLANG_PASS_THROUGH_SPIRV_OPT;
    return DownstreamCompilerUtil::findCompiler(
        ioSet,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

// Two minimal, well-formed SPIR-V headers. `link` forwards them verbatim, so the contents only
// matter to the fake linker, which echoes the first one back.
const uint32_t kSpirvModuleA[] = {0x07230203, 0x00010000, 0x00080001, 1, 0};
const uint32_t kSpirvModuleB[] = {0x07230203, 0x00010000, 0x00080001, 2, 0};

// Call `link` with two modules, the count that makes `slang-emit.cpp` reach this path at all.
SlangResult linkTwoModules(IDownstreamCompiler* compiler, ComPtr<IArtifact>& outArtifact)
{
    const uint32_t* modules[2] = {kSpirvModuleA, kSpirvModuleB};
    const uint32_t moduleSizes[2] = {
        uint32_t(SLANG_COUNT_OF(kSpirvModuleA)),
        uint32_t(SLANG_COUNT_OF(kSpirvModuleB))};
    return compiler->link(modules, moduleSizes, 2, outArtifact.writeRef());
}

} // namespace

// When the linker entry point is absent, `link` must say "not available" rather than call through a
// null pointer.
SLANG_UNIT_TEST(downstreamLinkReportsUnavailableWhenSymbolMissing)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeLinkerState::Absent));
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader);
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeLinkerWasCalled = false;
    ComPtr<IArtifact> artifact;
    const SlangResult result = linkTwoModules(compiler, artifact);

    SLANG_CHECK(result == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(!gFakeLinkerWasCalled);
    // A failed link must not hand back an artifact; `slang-emit.cpp` would `loadBlob` from it.
    SLANG_CHECK(artifact == nullptr);
}

// The control for the test above: a linker that is present and rejects the modules must keep
// returning `SLANG_FAIL`, so an unconditional `SLANG_E_NOT_AVAILABLE` cannot satisfy both tests.
SLANG_UNIT_TEST(downstreamLinkReportsFailWhenLinkerRejects)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeLinkerState::PresentRejecting));
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader);
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeLinkerWasCalled = false;
    ComPtr<IArtifact> artifact;
    const SlangResult result = linkTwoModules(compiler, artifact);

    SLANG_CHECK(result == SLANG_FAIL);
    SLANG_CHECK(result != SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(gFakeLinkerWasCalled);
    SLANG_CHECK(artifact == nullptr);
}

// The success path, which neither failure test covers: a linker that is present and accepts must
// produce an artifact.
SLANG_UNIT_TEST(downstreamLinkSucceedsWhenLinkerAccepts)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeLinkerState::PresentAccepting));
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader);
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeLinkerWasCalled = false;
    ComPtr<IArtifact> artifact;
    const SlangResult result = linkTwoModules(compiler, artifact);

    SLANG_CHECK(gFakeLinkerWasCalled);
    SLANG_CHECK(result == SLANG_OK);
    SLANG_CHECK(artifact != nullptr);
}

namespace
{

// What a compile against the fake library produced. The tests assert on the result and on whether
// any SPIR-V came back, not only on the diagnostic text: "was it reported" and "was unlinked code
// still handed to the caller" are separate properties.
struct FakeGlslangCompileOutcome
{
    SlangResult codeResult;
    bool producedCode;
    String diagnostics;
};

// Compile an entry point to SPIR-V against a fake `slang-glslang`, optionally precompiling the
// imported library module first.
//
// `precompileLibrary` is what decides whether the link path is reached at all:
// `precompileForTarget(SLANG_SPIRV)` attaches an `EmbeddedDownstreamIR` blob to the library module,
// which `slang-emit.cpp` collects as a second entry in `spirvFiles`, making `needsLink` true. With
// it false there is exactly one module and `link` is never called -- the control the
// single-module test relies on.
FakeGlslangCompileOutcome compileWithFakeGlslang(
    FakeLinkerState linkerState,
    bool precompileLibrary)
{
    gFakeLibraryWasLoaded = false;
    gFakeCompileWasCalled = false;
    gFakeLinkerWasCalled = false;

    const char* librarySource = R"SLANG(
        module lib;
        public int addOne(int x)
        {
            return x + 1;
        }
    )SLANG";

    const char* entryPointSource = R"SLANG(
        import lib;

        RWStructuredBuffer<int> gOutputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutputBuffer[dispatchThreadID.x] = addOne(int(dispatchThreadID.x));
        }
    )SLANG";

    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.createDirectory("root");
    memoryFS.saveFile("root/lib.slang", librarySource, strlen(librarySource));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(linkerState));
    globalSession->setSharedLibraryLoader(loader);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    const char* searchPaths[] = {"root"};
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.fileSystem = fs;
    sessionDesc.searchPathCount = 1;
    sessionDesc.searchPaths = searchPaths;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* library = session->loadModule("lib", diagnostics.writeRef());
    SLANG_CHECK_ABORT(library != nullptr);

    if (precompileLibrary)
    {
        // Precompiling is on a separate experimental interface rather than on `IModule` itself.
        ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
        SLANG_CHECK_ABORT(
            library->queryInterface(
                slang::IModulePrecompileService_Experimental::getTypeGuid(),
                (void**)precompileService.writeRef()) == SLANG_OK);
        SLANG_CHECK_ABORT(precompileService != nullptr);

        diagnostics.setNull();
        SLANG_CHECK_ABORT(
            precompileService->precompileForTarget(SLANG_SPIRV, diagnostics.writeRef()) ==
            SLANG_OK);
    }

    diagnostics.setNull();
    auto module = session->loadModuleFromSourceString(
        "entry",
        "entry.slang",
        entryPointSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* componentTypes[3] = {library, module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            componentTypes,
            3,
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    FakeGlslangCompileOutcome outcome;
    outcome.codeResult =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    outcome.producedCode = code && code->getBufferSize() != 0;

    if (diagnostics && diagnostics->getBufferSize())
    {
        outcome.diagnostics = String(
            (const char*)diagnostics->getBufferPointer(),
            (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
    }
    return outcome;
}

} // namespace

// The consumer half: linking two modules with the linker absent must name the missing capability
// rather than fail silently, which is what the guard alone would produce.
SLANG_UNIT_TEST(downstreamLinkingUnavailableDiagnosesMissingLinker)
{
    const FakeGlslangCompileOutcome outcome = compileWithFakeGlslang(FakeLinkerState::Absent, true);
    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();

    // Checked first so that a compile which reported nothing at all fails here, on the reason,
    // rather than further down on a missing substring.
    SLANG_CHECK(diagnosticSlice.getLength() != 0);

    // The diagnostic must come from the fake library being loaded and consulted, not from an
    // earlier failure that never got near the linker.
    SLANG_CHECK(gFakeLibraryWasLoaded);
    SLANG_CHECK(!gFakeLinkerWasCalled);

    // A link that could not run must fail the compile and yield no SPIR-V: handing back the
    // unlinked module would silently drop the imported code.
    SLANG_CHECK(SLANG_FAILED(outcome.codeResult));
    SLANG_CHECK(!outcome.producedCode);

    // Pin the severity and code, not just the wording: downgrading the diagnostic to a `warning(`
    // leaves every text assertion below passing.
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("error[E00116]")) >= 0);

    // The environment fault is named, and the shader is not blamed for it.
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_linkSPIRV")) >= 0);
}

// The other side of the caller's branch: a linker that is present and rejects the modules must not
// produce the "unavailable" diagnostic, so routing every failure to that message cannot satisfy
// both this test and the one above.
SLANG_UNIT_TEST(downstreamLinkRejectionDoesNotDiagnoseUnavailable)
{
    const FakeGlslangCompileOutcome outcome =
        compileWithFakeGlslang(FakeLinkerState::PresentRejecting, true);

    SLANG_CHECK(SLANG_FAILED(outcome.codeResult));
    SLANG_CHECK(!outcome.producedCode);
    SLANG_CHECK(gFakeLinkerWasCalled);

    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("E00116")) < 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_linkSPIRV")) < 0);
}

// The control that proves the diagnostic above is caused by the *link* path specifically. Without
// precompiling the library there is only one SPIR-V module, so `needsLink` is false and a missing
// linker is irrelevant -- yet the fake library is still loaded and used, because the default
// optimization level leaves `needsOptimization` true. So this compile must succeed and stay silent
// even though `glslang_linkSPIRV` is just as absent as in the test above: it rules out the
// diagnostic being triggered by merely loading a linker-less library.
SLANG_UNIT_TEST(downstreamLinkingUnavailableIsSilentForSingleModule)
{
    const FakeGlslangCompileOutcome outcome =
        compileWithFakeGlslang(FakeLinkerState::Absent, false);
    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();

    // The silence below is only meaningful if the linker-less library was actually loaded and used:
    // a compile that never reached the fake library would also succeed and say nothing.
    SLANG_CHECK(gFakeLibraryWasLoaded);
    SLANG_CHECK(gFakeCompileWasCalled);
    // ...and `link` was never reached, which is what makes the missing symbol irrelevant here.
    SLANG_CHECK(!gFakeLinkerWasCalled);

    SLANG_CHECK(SLANG_SUCCEEDED(outcome.codeResult));
    SLANG_CHECK(outcome.producedCode);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("E00116")) < 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_linkSPIRV")) < 0);
}
