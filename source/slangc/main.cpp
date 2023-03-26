// main.cpp

#include "../../slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"
#include "../core/slang-test-tool-util.h"

using namespace Slang;

#include <assert.h>

#ifdef _WIN32
#define MAIN slangc_main
#else
#define MAIN main
#endif

static void _diagnosticCallback(
    char const* message,
    void*       /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
}

static SlangResult _compile(SlangCompileRequest* compileRequest, int argc, const char*const* argv)
{
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);
    spSetCommandLineCompilerMode(compileRequest);

    char const* appName = "slangc";
    if (argc > 0) appName = argv[0];

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
        // Run the compiler (this will produce any diagnostics through SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        res = spCompile(compileRequest);
        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match previous behavior
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

SLANG_TEST_TOOL_API SlangResult innerMain(StdWriters* stdWriters, slang::IGlobalSession* sharedSession, int argc, const char*const* argv)
{
    StdWriters::setSingleton(stdWriters);

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded stdlib, is sharedSession is not nullptr.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(Index(argc - 1), argv + 1))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
    }
    else if (!session)
    {
        // Just create the global session in the regular way if there isn't one set
        SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()));
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
        for(int ii = 0; ii < argc; ++ii)
        {
            args.add(String::fromWString(argv[ii]));
        }
        List<char const*> argBuffers;
        for(int ii = 0; ii < argc; ++ii)
        {
            argBuffers.add(args[ii].getBuffer());
        }

        result = MAIN(argc, (char**) &argBuffers[0]);
    }

#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif

    return result;
}
#endif
