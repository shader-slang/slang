// main.cpp

#include "slang-tag-version.h"
#include "slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"
#include "../core/slang-test-tool-util.h"
#include "../slang/slang-internal.h"

using namespace Slang;

#include <assert.h>
#include <cctype>

// Get the slangc embedded version
static const char* getSlangcVersionString()
{
    return SLANG_TAG_VERSION;
}

// Check if version option is present and handle version check
static bool handleVersionOption(
    SlangCompileRequest* compileRequest,
    int argc,
    const char* const* argv)
{
    // Look for version options in command line
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0)
        {
            // Get slangc version
            const char* slangcVersionRaw = getSlangcVersionString();

            // Get slang library version
            const char* slangLibVersion = spGetBuildTagString();

            // Apply the same logic as getBuildTagString() for slangc version
            String slangcVersion;
            if (UnownedStringSlice(slangcVersionRaw) == "0.0.0-unknown")
            {
                // For slangc, we can't easily get the executable timestamp like shared libraries,
                // so we'll use a different approach - we'll consider "0.0.0-unknown" to match
                // any library version that also resolves to a timestamp.
                slangcVersion = slangcVersionRaw;
            }
            else
            {
                slangcVersion = slangcVersionRaw;
            }

            // Compare versions
            bool versionsMatch = false;
            if (slangcVersion == slangLibVersion)
            {
                versionsMatch = true;
            }
            else if (
                UnownedStringSlice(slangcVersionRaw) == "0.0.0-unknown" &&
                UnownedStringSlice(slangLibVersion).getLength() > 0 && isdigit(slangLibVersion[0]))
            {
                // Both are using fallback logic (timestamp for library, unknown for slangc)
                // Consider this a match since they're built from the same source
                versionsMatch = true;
            }

            auto stdOut = StdWriters::getOut();
            stdOut.print("%s\n", slangLibVersion);

            if (!versionsMatch)
            {
                // Versions don't match, print warning
                auto stdError = StdWriters::getError();
                stdError.print(
                    "warning: slangc version (%s) does not match slang library version (%s)\n",
                    slangcVersion.getBuffer(),
                    slangLibVersion);
            }
            return true; // Handled version option
        }
    }
    return false; // No version option found
}

#ifdef _WIN32
#define MAIN slangc_main
#else
#define MAIN main
#endif

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
}

static SlangResult _compile(SlangCompileRequest* compileRequest, int argc, const char* const* argv)
{
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);
    spSetCommandLineCompilerMode(compileRequest);

    char const* appName = "slangc";
    if (argc > 0)
        appName = argv[0];

    // Check for version option first and handle version checking
    if (handleVersionOption(compileRequest, argc, argv))
    {
        return SLANG_OK; // Version option handled, exit successfully
    }

    {
        const SlangResult res = spProcessCommandLineArguments(compileRequest, &argv[1], argc - 1);
        if (SLANG_FAILED(res))
        {
            // TODO: print usage message
            return res;
        }
    }

    SlangResult res = SLANG_OK;

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through
        // SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        res = spCompile(compileRequest);
        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match
        // previous behavior
        res = SLANG_FAILED(res) ? SLANG_E_INTERNAL_FAIL : res;
    }
#ifndef _DEBUG
    catch (const Exception& e)
    {
        StdWriters::getOut().print("internal compiler error: %S\n", e.Message.toWString().begin());
        res = SLANG_FAIL;
    }
#endif

    return res;
}

bool shouldEmbedPrelude(const char* const* argv, int argc)
{
    for (int i = 0; i < argc; i++)
    {
        if (UnownedStringSlice(argv[i]) == "-embed-prelude")
            return true;
    }
    return false;
}

SLANG_TEST_TOOL_API SlangResult innerMain(
    StdWriters* stdWriters,
    slang::IGlobalSession* sharedSession,
    int argc,
    const char* const* argv)
{
    StdWriters::setSingleton(stdWriters);

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded core module, is sharedSession is not nullptr.
    // This differed test checks if the command line has an option to setup the core module.
    // If so we *don't* use the sharedSession, and create a new session without the core module just
    // for this compilation.
    if (TestToolUtil::hasDeferredCoreModule(Index(argc - 1), argv + 1))
    {
        SLANG_RETURN_ON_FAIL(
            slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, session.writeRef()));
    }
    else if (!session)
    {
        // Just create the global session in the regular way if there isn't one set
        SlangGlobalSessionDesc desc = {};
        desc.enableGLSL = true;
        Slang::GlobalSessionInternalDesc internalDesc = {};
#ifdef SLANG_BOOTSTRAP
        internalDesc.isBootstrap = true;
#endif
        SLANG_RETURN_ON_FAIL(
            slang_createGlobalSessionImpl(&desc, &internalDesc, session.writeRef()));
    }

    if (!shouldEmbedPrelude(argv, argc))
        TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], session);

    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);
    compileRequest->addSearchPath(Path::getParentDirectory(Path::getExecutablePath()).getBuffer());
    SlangResult res = _compile(compileRequest, argc, argv);
    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);

    return res;
}

int MAIN(int argc, char** argv)
{
    auto stdWriters = StdWriters::initDefaultSingleton();
    SlangResult res = innerMain(stdWriters, nullptr, argc, argv);
    slang::shutdown();
    return (int)TestToolUtil::getReturnCode(res);
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    int result = 0;

    {
        // Convert the wide-character Unicode arguments to UTF-8,
        // since that is what Slang expects on the API side.

        List<String> args;
        for (int ii = 0; ii < argc; ++ii)
        {
            args.add(String::fromWString(argv[ii]));
        }
        List<char const*> argBuffers;
        for (int ii = 0; ii < argc; ++ii)
        {
            argBuffers.add(args[ii].getBuffer());
        }

        result = MAIN(argc, (char**)&argBuffers[0]);
    }

#ifdef _MSC_VER
    // _CrtXXX functions are functional only for debug build. The spec says,
    // "When _DEBUG isn't defined, calls to _CrtSetReportMode are removed
    // during preprocessing."
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    int memleakDetected = _CrtDumpMemoryLeaks();
    SLANG_UNUSED(memleakDetected);
    assert(!memleakDetected);
#endif

    return result;
}
#endif
