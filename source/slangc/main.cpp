// main.cpp

#include "../../slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"
#include "../core/slang-app-context.h"
#include "../core/slang-writer.h"

using namespace Slang;

#include <assert.h>

static void diagnosticCallback(
    char const* message,
    void*       /*userData*/)
{
    auto stdError = AppContext::getStdError();
    stdError.put(message);
    stdError.flush();
}

#ifdef _WIN32
#define MAIN slangc_main
#else
#define MAIN main
#endif

SLANG_SHARED_LIBRARY_TOOL_API SlangResult innerMain(AppContext* appContext, SlangSession* session, int argc, const char*const* argv)
{
    AppContext::setSingleton(appContext);

    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);

    spSetDiagnosticCallback(
        compileRequest,
        &diagnosticCallback,
        nullptr);

    spSetCommandLineCompilerMode(compileRequest);

    // Do any app specific configuration
    appContext->configureRequest(compileRequest);

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
    catch (Exception & e)
    {
        AppContext::getStdOut().print("internal compiler error: %S\n", e.Message.ToWString().begin());
        res = SLANG_FAIL;
    }
#endif

    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);
    return res;
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
        // Convert the wide-character Unicode arguments to UTF-8,
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
