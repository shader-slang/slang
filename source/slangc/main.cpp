// main.cpp

#include "../../slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"
#include "../core/slang-app-context.h"

using namespace Slang;

#include <assert.h>

// Try to read an argument for a command-line option.

static void outputCallback(
    char const* message,
    void*       userData)
{
    auto stdError = (WriteStream * )userData;
    
    stdError->put(message);
    stdError->flush();
}

#ifdef _WIN32
#define MAIN slangc_main
#else
#define MAIN main
#endif

SLANG_SHARED_LIBRARY_TOOL_API SlangResult innerMain(AppContext* appContext, SlangSession* session, int argc, const char*const* argv)
{
    // Parse any command-line options
    AppContext::setSingleton(appContext);

    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);

    spSetDiagnosticCallback(
        compileRequest,
        &outputCallback,
        AppContext::getStdError());

    if (!AppContext::getStdOut()->isFile())
    {
        spSetOutputCallback(
            compileRequest,
            &outputCallback,
            AppContext::getStdOut());
    }

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

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through
        // our callback above).
        if (SLANG_FAILED(spCompile(compileRequest)))
        {
            // If the compilation failed, then get out of here...
            // Turn into an internal Result -> such that return code can be used to vary result to match previous behavior
            return SLANG_E_INTERNAL_FAIL;
        }

        // Now that we are done, clean up after ourselves

        spDestroyCompileRequest(compileRequest);

    }
#ifndef _DEBUG
    catch (Exception & e)
    {
        AppContext::getStdOut()->print("internal compiler error: %S\n", e.Message.ToWString().begin());
        return SLANG_FAIL;
    }
#endif
    return SLANG_OK;    
}

int MAIN(int argc, char** argv)
{
    SlangResult res;
    {
        SlangSession* session = spCreateSession(nullptr);
        res = innerMain(AppContext::initDefault(), session, argc, argv);
        spDestroySession(session);
    }
    return AppContext::getReturnCode(res);
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    int result = 0;

    {
        // Conver the wide-character Unicode arguments to UTF-8,
        // since that is what Slang expects on the API side.

        List<String> args;
        for(int ii = 0; ii < argc; ++ii)
        {
            args.Add(String::FromWString(argv[ii]));
        }
        List<char const*> argBuffers;
        for(int ii = 0; ii < argc; ++ii)
        {
            argBuffers.Add(args[ii].Buffer());
        }

        result = MAIN(argc, (char**) &argBuffers[0]);
    }

#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif

    return result;
}
#endif
