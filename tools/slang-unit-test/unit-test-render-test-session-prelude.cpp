// unit-test-render-test-session-prelude.cpp

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-shared-library.h"
#include "core/slang-std-writers.h"
#include "core/slang-string-util.h"
#include "core/slang-test-tool-util.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{ // anonymous

typedef SlangResult (*InnerMainFunc)(Slang::StdWriters*, SlangSession*, int, const char* const*);

static String _getLanguagePrelude(slang::IGlobalSession* session, SlangSourceLanguage language)
{
    ComPtr<ISlangBlob> blob;
    session->getLanguagePrelude(language, blob.writeRef());
    // An empty blob can carry a null buffer pointer, so do not do pointer arithmetic on it.
    if (!blob || blob->getBufferSize() == 0)
        return String();
    return String(
        (const char*)blob->getBufferPointer(),
        (const char*)blob->getBufferPointer() + blob->getBufferSize());
}

/// Returns an empty string on failure.
static String _createProbeShader()
{
    // `generateTemporary` creates the file it names, so use that path rather than deriving another.
    String path;
    if (SLANG_FAILED(File::generateTemporary(toSlice("render-test-prelude-probe"), path)))
        return String();

    if (SLANG_FAILED(File::writeAllText(
            path,
            "[shader(\"compute\")]\n"
            "[numthreads(1, 1, 1)]\n"
            "void computeMain(uint3 tid : SV_DispatchThreadID)\n"
            "{\n"
            "}\n")))
    {
        File::remove(path);
        return String();
    }
    return path;
}

/// Deletes a file when it goes out of scope, so an aborting assertion cannot leak it.
struct ScopedFile
{
    explicit ScopedFile(String path)
        : m_path(path)
    {
    }
    ~ScopedFile() { File::remove(m_path); }

    ScopedFile(const ScopedFile&) = delete;
    ScopedFile& operator=(const ScopedFile&) = delete;

    String m_path;
};

/// Returns an empty string if the module is in neither supported location.
///
/// Windows keeps tool modules beside the executable and other platforms use a sibling `lib`, but a
/// WebGPU-enabled build overrides that and places render-test with the runtime sidecar libraries
/// instead (see `tools/render-test/CMakeLists.txt`).
static String _findRenderTestModule(const char* executableDirectory)
{
    StringBuilder fileName;
    SharedLibrary::appendPlatformFileName(toSlice("render-test-tool"), fileName);

    List<String> candidates;
    String dllDirectory;
    if (SLANG_SUCCEEDED(TestToolUtil::getDllDirectoryPath(
            Path::combine(executableDirectory, "slang-test").getBuffer(),
            dllDirectory)))
    {
        candidates.add(dllDirectory);
    }
    candidates.add(String(executableDirectory));

    for (const auto& directory : candidates)
    {
        String path = Path::combine(directory, fileName.produceString());
        if (File::exists(path))
            return path;
    }
    return String();
}

} // anonymous namespace

// render-test is given a session it does not own — slang-test and the test server each create one
// global session per process and pass the same one to every tool invocation — and it installs an
// HLSL prelude on it. Any prelude it fails to restore applies to every later compile in that
// process, so this test acts as such a caller and requires its prelude back unchanged.
SLANG_UNIT_TEST(renderTestSessionPrelude)
{
    // `-nvapi-extn-slot` is deliberately absent below: its absence is what selects the branch of
    // `_setSessionPrelude` that overwrites the caller's HLSL prelude.
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(session.writeRef())));

    // Sentinels rather than the real preludes, so a restore that happens to install something
    // resembling the default cannot pass.
    const char* const sentinelHlslPrelude = "// slang#12442 sentinel HLSL prelude\n";
    const char* const sentinelCppPrelude = "// slang#12442 sentinel CPP prelude\n";
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, sentinelHlslPrelude);
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_CPP, sentinelCppPrelude);

    // Not `unitTestContext->workDirectory`: slang-test sets it to the empty string, so writing
    // there would drop the probe shader into the repository root.
    String shaderPath = _createProbeShader();
    SLANG_CHECK_ABORT(shaderPath.getLength() != 0);
    ScopedFile shaderFile(shaderPath);

    String sharedLibPath = _findRenderTestModule(unitTestContext->executableDirectory);
    if (sharedLibPath.getLength() == 0)
    {
        getTestReporter()->message(
            TestMessageType::TestFailure,
            "could not locate the render-test-tool module");
    }
    SLANG_CHECK_ABORT(sharedLibPath.getLength() != 0);

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(DefaultSharedLibraryLoader::getSingleton()->loadPlatformSharedLibrary(
            sharedLibPath.getBuffer(),
            sharedLibrary.writeRef())));

    auto innerMain = (InnerMainFunc)sharedLibrary->findFuncByName("innerMain");
    SLANG_CHECK_ABORT(innerMain != nullptr);

    // A `-cpu` compute run: `-compile-only` keeps it off the GPU and away from downstream compilers
    // so the test runs anywhere, while still reaching `_setSessionPrelude`, which is the code under
    // test.
    List<const char*> args;
    args.add("render-test");
    args.add(shaderPath.getBuffer());
    args.add("-cpu");
    args.add("-compute");
    args.add("-compile-only");
    args.add("-entry");
    args.add("computeMain");

    // render-test installs these writers as its own module's `StdWriters` singleton and does not
    // put the previous one back, so they have to outlive every later use of the module. Keeping the
    // reference alive for the rest of the test is enough here, since nothing calls into it again.
    RefPtr<StdWriters> stdWriters = StdWriters::createDefault();
    SlangResult innerMainResult =
        innerMain(stdWriters, session, (int)args.getCount(), args.getBuffer());

    // The run has to be checked even though the prelude is what this test is about. render-test
    // installs the prelude late enough that a run which bails out earlier — an unrecognized option,
    // say — never touches it, and then the prelude below is trivially unchanged and the test passes
    // while having exercised nothing.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(innerMainResult));

    SLANG_CHECK(_getLanguagePrelude(session, SLANG_SOURCE_LANGUAGE_HLSL) == sentinelHlslPrelude);
    SLANG_CHECK(_getLanguagePrelude(session, SLANG_SOURCE_LANGUAGE_CPP) == sentinelCppPrelude);

    // Repeat with an empty prelude, which is the case that reaches the guard's zero-size blob path.
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_HLSL, "");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(innerMain(stdWriters, session, (int)args.getCount(), args.getBuffer())));
    SLANG_CHECK(_getLanguagePrelude(session, SLANG_SOURCE_LANGUAGE_HLSL).getLength() == 0);
}
