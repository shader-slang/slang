// unit-test-nvrtc-pch-invalidation.cpp

#include "compiler-core/slang-artifact-associated.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-artifact.h"
#include "compiler-core/slang-downstream-compiler-set.h"
#include "compiler-core/slang-downstream-compiler.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "core/slang-blob.h"
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
    String& outState)
{
    outState = String();

    ComPtr<IArtifact> sourceArtifact = ArtifactUtil::createArtifact(
        ArtifactDescUtil::makeDescForSourceLanguage(SLANG_SOURCE_LANGUAGE_CUDA));
    sourceArtifact->addRepresentationUnknown(StringBlob::create(source));

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
    options.targetType = SLANG_PTX;
    IArtifact* sourceArtifacts[] = {sourceArtifact.get()};
    options.sourceArtifacts = makeSlice(sourceArtifacts, 1);

    ComPtr<IArtifact> artifact;
    SlangResult compileRes = compiler->compile(options, artifact.writeRef());

    if (artifact)
    {
        if (auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact))
        {
            const char* raw = diagnostics->getRaw().begin();
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
    }
    return compileRes;
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

    // Negative branch of the gate: a source that does not begin with `#include` must NOT get
    // `-pch`, so no status marker is emitted. This guards against the gate regressing to always-on
    // (which would request `-pch` for the verbatim/embedded prelude, where it cannot help).
    String verbatimState;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, String(kernel), verbatimState)));
    SLANG_CHECK(verbatimState.getLength() == 0);

    // slang-test may retry a failed unit test in the same process, and the PCH heap is
    // process-global, so a fixed key would already exist on a retry and the first compile would
    // report "not-created" instead of "created". Derive a per-run nonce so every run — including a
    // retry in this process — uses a key NVRTC has never seen.
    static int s_runCounter = 0;
    const int runNonce = ++s_runCounter;

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

    // First compile of source A: NVRTC has no header for this key yet, so it creates one.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compiler, sourceA, state)));
    // A libnvrtc that reports >= 12.8 but lacks the nvrtcGetPCHCreateStatus symbol emits no marker.
    // The create/reuse signal is then unobservable, so skip rather than report a spurious failure.
    if (state.getLength() == 0)
    {
        SLANG_IGNORE_TEST;
    }
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
