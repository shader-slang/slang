// unit-test-nvrtc-pch-invalidation.cpp

#include "compiler-core/slang-artifact-associated.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-artifact.h"
#include "compiler-core/slang-downstream-compiler-set.h"
#include "compiler-core/slang-downstream-compiler.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "compiler-core/slang-slice-allocator.h"
#include "core/slang-blob.h"
#include "core/slang-castable.h"
#include "core/slang-io.h"
#include "core/slang-shared-library.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// The NVRTC driver appends "slang-nvrtc-pch-status: <state>" to a compile's raw diagnostics when it
// requested `-pch`, where <state> is "created" (a precompiled header was built this compile),
// "not-created" (none was built — an existing one was reused, or the compiler declined), or
// "create-failed" (creation was attempted but failed). The text is Slang-owned, so this test does
// not depend on NVRTC's own log wording. NVRTC does not report reuse directly; this test infers it
// from a "not-created" that follows a "created" for the same key.
static const char* kPchStatusMarker = "slang-nvrtc-pch-status: ";

// Compile a CUDA source to PTX through the NVRTC downstream compiler and read back the PCH status
// token (empty if the marker is absent, e.g. `-pch` was not requested). Returns the compile result.
static SlangResult compileCudaSource(
    IDownstreamCompiler* compiler,
    const String& source,
    String& outState,
    const char* name = nullptr,
    const List<String>* arguments = nullptr,
    String* outLog = nullptr,
    ISlangBlob** outCode = nullptr)
{
    outState = String();

    ComPtr<IArtifact> sourceArtifact = ArtifactUtil::createArtifact(
        ArtifactDescUtil::makeDescForSourceLanguage(SLANG_SOURCE_LANGUAGE_CUDA));
    if (name)
        sourceArtifact->setName(name);
    sourceArtifact->addRepresentationUnknown(StringBlob::create(source));

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
    options.targetType = SLANG_PTX;
    IArtifact* sourceArtifacts[] = {sourceArtifact.get()};
    options.sourceArtifacts = makeSlice(sourceArtifacts, 1);
    SliceAllocator allocator;
    if (arguments)
        options.compilerSpecificArguments = allocator.allocate(*arguments);

    ComPtr<IArtifact> artifact;
    SlangResult compileRes = compiler->compile(options, artifact.writeRef());

    if (artifact)
    {
        if (auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact))
        {
            const char* raw = diagnostics->getRaw().begin();
            if (outLog)
                *outLog = raw ? String(raw) : String();
            if (raw)
            {
                if (const char* p = strstr(raw, kPchStatusMarker))
                {
                    p += strlen(kPchStatusMarker);
                    const char* e = p;
                    while (*e && *e != '\n')
                    {
                        ++e;
                    }
                    outState.append(UnownedStringSlice(p, e));
                }
            }
            if (SLANG_SUCCEEDED(compileRes) && SLANG_FAILED(diagnostics->getResult()))
                compileRes = diagnostics->getResult();
        }
        if (outCode && SLANG_SUCCEEDED(compileRes))
            compileRes = artifact->loadBlob(ArtifactKeep::Yes, outCode);
    }
    return compileRes;
}

// Create another real adapter without calling the hidden compiler lookup in libslang-compiler.
// Keeping the returned interface alive keeps the compiler after this temporary set is destroyed.
static SlangResult loadPchTestCompiler(ComPtr<IDownstreamCompiler>& outCompiler)
{
    RefPtr<DownstreamCompilerSet> set(new DownstreamCompilerSet());
    SLANG_RETURN_ON_FAIL(NVRTCDownstreamCompilerUtil::locateCompilers(
        String(),
        DefaultSharedLibraryLoader::getSingleton(),
        set));
    List<IDownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    if (compilers.getCount() == 0)
        return SLANG_E_NOT_AVAILABLE;
    outCompiler = compilers[0];
    return SLANG_OK;
}

struct PchOwnershipTestContext
{
    String source;
    ComPtr<ISlangSharedLibrary> externalLibrary;
    ComPtr<IDownstreamCompiler> compiler;

    // Decide capability skips before any checks. Compilation failures after this point are test
    // failures, including inability to acquire an automatic-PCH directory.
    bool init(slang::IGlobalSession* session)
    {
        ComPtr<slang::IBlob> prelude;
        session->getLanguagePrelude(SLANG_SOURCE_LANGUAGE_CUDA, prelude.writeRef());
        if (!prelude)
            return false;
        source = String(
            UnownedStringSlice((const char*)prelude->getBufferPointer(), prelude->getBufferSize()));
        if (!source.getUnownedSlice().trimStart().startsWith("#include") ||
            SLANG_FAILED(loadPchTestCompiler(compiler)) ||
            compiler->getDesc().getVersionValue() < 1208)
            return false;
        ComPtr<ISlangBlob> path;
        auto provider = as<IDownstreamCompilerPathProvider>(compiler.get());
        if (!provider || SLANG_FAILED(provider->getPath(path.writeRef())) ||
            SLANG_FAILED(DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(
                (const char*)path->getBufferPointer(),
                externalLibrary.writeRef())) ||
            !externalLibrary->findFuncByName("nvrtcGetPCHCreateStatus"))
            return false;
        source.append("#define SLANG_NVRTC_PCH_OWNERSHIP_TEST 1\n"
                      "extern \"C\" __global__ void computeMain() {}\n");
        return true;
    }
};

// NVRTC's documented default PCH messages include the selected file in quotes. Read that path to
// check ownership without adding a production API or diagnostic solely to expose private state.
// A changed message format fails the test instead of silently dropping its filesystem assertions.
static String getPchFileFromLog(const String& log)
{
    const char* start = log.getBuffer();
    const char* end = strstr(start, ".pch\"");
    if (!end)
        return String();
    const char* begin = end;
    while (begin > start && begin[-1] != '"')
        --begin;
    return begin > start ? String(UnownedStringSlice(begin, end + 4)) : String();
}

struct PchTestDirectory
{
    String path;
    ~PchTestDirectory()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

SLANG_UNIT_TEST(nvrtcPrecompiledHeaderOwnership)
{
    PchOwnershipTestContext context;
    if (!context.init(unitTestContext->slangGlobalSession))
    {
        SLANG_IGNORE_TEST;
    }

    String state;
    String log;
    ComPtr<ISlangBlob> firstCode;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(
        context.compiler,
        context.source,
        state,
        "retained-name.cu",
        nullptr,
        &log,
        firstCode.writeRef())));
    SLANG_CHECK(state == "created");
    String firstFile = getPchFileFromLog(log);
    SLANG_CHECK_ABORT(firstFile.getLength() && File::exists(firstFile));
    String firstDirectory = Path::getParentDirectory(firstFile);

    ComPtr<ISlangBlob> repeatedCode;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(
        context.compiler,
        context.source,
        state,
        "retained-name.cu",
        nullptr,
        &log,
        repeatedCode.writeRef())));
    SLANG_CHECK(state == "not-created");
    SLANG_CHECK_ABORT(firstCode->getBufferSize() == repeatedCode->getBufferSize());
    SLANG_CHECK(
        memcmp(
            firstCode->getBufferPointer(),
            repeatedCode->getBufferPointer(),
            firstCode->getBufferSize()) == 0);

    ComPtr<IDownstreamCompiler> second;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadPchTestCompiler(second)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileCudaSource(second, context.source, state, "retained-name.cu", nullptr, &log)));
    SLANG_CHECK(state == "created");
    String secondFile = getPchFileFromLog(log);
    SLANG_CHECK_ABORT(secondFile.getLength() && File::exists(secondFile));
    String secondDirectory = Path::getParentDirectory(secondFile);
    SLANG_CHECK(firstDirectory != secondDirectory);

    // Destroy the first adapter while both the second adapter and an external library reference
    // remain alive. Its files disappear, while the second adapter still reuses its own PCH.
    context.compiler.setNull();
    SLANG_CHECK(!File::exists(firstDirectory));
    SLANG_CHECK(File::exists(secondFile));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileCudaSource(second, context.source, state, "retained-name.cu", nullptr, &log)));
    SLANG_CHECK(state == "not-created");

    String invalidSource = context.source + "\nthis_is_a_syntax_error\n";
    SLANG_CHECK(SLANG_FAILED(
        compileCudaSource(second, invalidSource, state, "retained-name.cu", nullptr, &log)));
    SLANG_CHECK(strstr(log.getBuffer(), "retained-name.cu") != nullptr);
    second.setNull();
    SLANG_CHECK(!File::exists(secondDirectory));
}

SLANG_UNIT_TEST(nvrtcPrecompiledHeaderCallerDirectory)
{
    PchOwnershipTestContext context;
    if (!context.init(unitTestContext->slangGlobalSession))
    {
        SLANG_IGNORE_TEST;
    }

    // NVRTC documents the equals-value spelling. Separate argv entries for the option and value
    // are rejected by NVRTC 12.9, so they are not a positive ownership/reuse contract.
    const char* aliases[] = {"--pch-dir=", "-pch-dir="};
    for (const char* alias : aliases)
    {
        PchTestDirectory directory;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            Path::createTemporaryDirectory(toSlice("slang-pch-caller"), directory.path)));
        String sentinel = Path::combine(directory.path, "caller-owned");
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(sentinel, "keep")));
        List<String> arguments;
        arguments.add(String(alias) + directory.path);

        ComPtr<IDownstreamCompiler> compiler;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadPchTestCompiler(compiler)));
        String state;
        String log;
        SlangResult compileResult =
            compileCudaSource(compiler, context.source, state, "caller-name.cu", &arguments, &log);
        if (SLANG_FAILED(compileResult))
        {
            getTestReporter()->message(TestMessageType::Info, arguments[0].getBuffer());
            getTestReporter()->message(TestMessageType::Info, log.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK(state == "created");
        String pchFile = getPchFileFromLog(log);
        SLANG_CHECK_ABORT(pchFile.getLength() && File::exists(pchFile));
        SLANG_CHECK(Path::equals(Path::getParentDirectory(pchFile), directory.path));
        compileResult =
            compileCudaSource(compiler, context.source, state, "caller-name.cu", &arguments, &log);
        if (SLANG_FAILED(compileResult))
        {
            getTestReporter()->message(TestMessageType::Info, arguments[0].getBuffer());
            getTestReporter()->message(TestMessageType::Info, log.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK(state == "not-created");
        compiler.setNull();
        SLANG_CHECK(File::exists(directory.path) && File::exists(sentinel));
        SLANG_CHECK(File::exists(pchFile));
    }

    // A missing value must still reach NVRTC unchanged. The adapter must not append a second
    // directory option that hides the malformed caller option or adopts caller-owned storage.
    for (const char* alias : {"--pch-dir", "-pch-dir"})
    {
        List<String> malformedArguments;
        malformedArguments.add(alias);
        String state;
        String log;
        SLANG_CHECK(SLANG_FAILED(compileCudaSource(
            context.compiler,
            context.source,
            state,
            "caller-name.cu",
            &malformedArguments,
            &log)));
        SLANG_CHECK(strstr(log.getBuffer(), alias) != nullptr);
        SLANG_CHECK(strstr(log.getBuffer(), "error") != nullptr);
    }
}

// Positively verify NVRTC's automatic precompiled header for the CUDA prelude: a header is created
// on the first compile, reused (not recreated) by an identical second compile, and rebuilt when the
// prelude's leading directive text changes. NVRTC exposes this only via nvrtcGetPCHCreateStatus,
// which the driver surfaces as a status token; the emitted PTX is identical throughout, so this
// cannot be observed through getEntryPointCode (that is what unit-test-nvrtc-pch.cpp guards).
// Requires a loadable NVRTC 12.8 or newer (where the driver adds `-pch`) and the include-form
// prelude; Ignored otherwise. NVRTC compiles to PTX without a GPU, so no device is needed.
SLANG_UNIT_TEST(nvrtcPrecompiledHeaderInvalidation)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    // The driver requests `-pch` only when the prelude reaches NVRTC as a leading `#include`. Use
    // the prelude the session actually installed; if it is not the include form (e.g. the embedded
    // default), `-pch` never engages and there is nothing to observe.
    ComPtr<slang::IBlob> preludeBlob;
    globalSession->getLanguagePrelude(SLANG_SOURCE_LANGUAGE_CUDA, preludeBlob.writeRef());
    UnownedStringSlice prelude = preludeBlob ? UnownedStringSlice(
                                                   (const char*)preludeBlob->getBufferPointer(),
                                                   preludeBlob->getBufferSize())
                                             : UnownedStringSlice();
    if (!prelude.trimStart().startsWith("#include"))
    {
        SLANG_IGNORE_TEST;
    }

    // Load NVRTC directly through compiler-core, which is statically linked into this tool. The
    // session's getOrLoadDownstreamCompiler lives in libslang-compiler.so with hidden visibility
    // and is not linkable here, so we locate the same libnvrtc ourselves.
    RefPtr<DownstreamCompilerSet> compilerSet(new DownstreamCompilerSet());
    if (SLANG_FAILED(NVRTCDownstreamCompilerUtil::locateCompilers(
            String(),
            DefaultSharedLibraryLoader::getSingleton(),
            compilerSet)))
    {
        SLANG_IGNORE_TEST;
    }
    List<IDownstreamCompiler*> compilers;
    compilerSet->getCompilers(compilers);
    if (compilers.getCount() == 0)
    {
        SLANG_IGNORE_TEST;
    }
    IDownstreamCompiler* compiler = compilers[0];

    // `-pch` is only added on NVRTC 12.8+. getVersionValue() is major*100 + minor. Gate on the
    // compiler actually exercised below (not the session's), so the version matches what is used.
    if (compiler->getDesc().getVersionValue() < 1208)
    {
        SLANG_IGNORE_TEST;
    }

    const char* kernel = "\nextern \"C\" __global__ void computeMain() {}\n";

    // slang-test may retry a failed unit test in the same process, and the PCH heap is
    // process-global, so a fixed key would already exist on a retry and the first compile would
    // report "not-created" instead of "created". Derive a per-run nonce so every run — including a
    // retry in this process — uses keys NVRTC has never seen.
    static int s_runCounter = 0;
    const int runNonce = ++s_runCounter;

    // Probe marker availability with a throwaway key BEFORE any SLANG_CHECK. A libnvrtc that
    // reports
    // >= 12.8 but lacks the nvrtcGetPCHCreateStatus symbol emits no marker, leaving the
    // create/reuse signal unobservable; skip in that case. This probe must precede the first
    // SLANG_CHECK because TestReporter::combine is max() over Ignored(0) < Pass(1): a
    // SLANG_IGNORE_TEST reached after a passing check would be masked into an overall Pass,
    // silently degrading this test to a green no-op. The probe uses its own key so it does not
    // pre-create the header for A/B below.
    {
        StringBuilder probeSource;
        probeSource << prelude << "#define SLANG_NVRTC_PCH_TEST_" << runNonce << "_PROBE 1\n"
                    << kernel;
        String probeState;
        if (SLANG_FAILED(compileCudaSource(compiler, probeSource, probeState)) ||
            probeState.getLength() == 0)
        {
            SLANG_IGNORE_TEST;
        }
    }

    // Negative branch of the gate: a source that does not begin with `#include` must NOT get
    // `-pch`, so no status marker is emitted. This guards against the gate regressing to always-on
    // (which would request `-pch` for the verbatim/embedded prelude, where it cannot help). It is
    // the first SLANG_CHECK, so every skip decision above has already been made.
    String verbatimState;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, String(kernel), verbatimState)));
    SLANG_CHECK(verbatimState.getLength() == 0);

    // Two distinct sources: each is the installed include-form prelude plus a nonce-unique extra
    // directive in NVRTC's leading-directive region (appended after the prelude, still before the
    // header stop point). The two distinct keys (a) guarantee no earlier compile in this process
    // created a header for either, so the first compile of each must create one, and (b) model the
    // leading directive text changing between compiles — what NVRTC keys its precompiled header on.
    // Both still begin with the `#include`, so the driver requests `-pch` for each.
    StringBuilder preludeA;
    preludeA << prelude << "#define SLANG_NVRTC_PCH_TEST_" << runNonce << "_A 1\n";
    StringBuilder preludeB;
    preludeB << prelude << "#define SLANG_NVRTC_PCH_TEST_" << runNonce << "_B 1\n";

    StringBuilder sourceA;
    sourceA << preludeA << kernel;
    StringBuilder sourceB;
    sourceB << preludeB << kernel;

    String state;

    // First compile of source A: NVRTC has no header for this key yet, so it creates one. (The
    // probe above already established that the status marker is available, so it must be non-empty
    // here.)
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, sourceA, state)));
    SLANG_CHECK(state.getUnownedSlice() == "created");

    // Identical second compile: NVRTC does not build a new header. Because the first compile
    // created one for this exact key, "not-created" here means that header was reused.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, sourceA, state)));
    SLANG_CHECK(state.getUnownedSlice() == "not-created");

    // Changed source (B): the leading directive text differs, so the previous header does not
    // apply and NVRTC builds a new one — the invalidation-on-change this test verifies.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, sourceB, state)));
    SLANG_CHECK(state.getUnownedSlice() == "created");
}
