// main.cpp

#define SLANG_DYNAMIC
#include "../../slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"

using namespace Slang;

#include <assert.h>

// Try to read an argument for a command-line option.

static void diagnosticCallback(
    char const* message,
    void*       /*userData*/)
{
    fputs(message, stderr);
    fflush(stderr);
}

#ifdef _WIN32
#define MAIN slangc_main
#else
#define MAIN main
#endif

int MAIN(int argc, char** argv)
{
    // Parse any command-line options

    SlangSession* session = spCreateSession(nullptr);
    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);

    spSetDiagnosticCallback(
        compileRequest,
        &diagnosticCallback,
        nullptr);

    spSetCommandLineCompilerMode(compileRequest);

    char const* appName = "slangc";
    if(argc > 0) appName = argv[0];

    int err = spProcessCommandLineArguments(compileRequest, &argv[1], argc - 1);
    if( err )
    {
        // TODO: print usage message
        exit(1);
    }

    // Invoke the compiler

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through
        // our callback above).
        int result = spCompile(compileRequest);
        if( result != 0 )
        {
            // If the compilation failed, then get out of here...
            exit(-1);
        }

        // Now that we are done, clean up after ourselves

        spDestroyCompileRequest(compileRequest);
        spDestroySession(session);
    }
#ifndef _DEBUG
    catch (Exception & e)
    {
        printf("internal compiler error: %S\n", e.Message.ToWString().begin());
        return 1;
    }
#endif

    return 0;
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
