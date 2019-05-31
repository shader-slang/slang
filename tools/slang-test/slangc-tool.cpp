// test-context.cpp
#include "slangc-tool.h"

#include "../../source/core/slang-exception.h"

using namespace Slang;

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
}

SlangResult SlangCTool::innerMain(StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv)
{
    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);

    spSetCommandLineCompilerMode(compileRequest);
    // Do any app specific configuration
    stdWriters->setRequestWriters(compileRequest);

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
        StdWriters::getOut().print("internal compiler error: %S\n", e.Message.toWString().begin());
        res = SLANG_FAIL;
    }
#endif

    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);
    return res;
}

