// test-context.cpp
#include "slangc-tool.h"

#include "../../source/core/slang-exception.h"
#include "../../source/core/slang-test-tool-util.h"

using namespace Slang;

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
}

static SlangResult _compile(SlangCompileRequest* compileRequest, int argc, const char*const* argv)
{
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);
    spSetCommandLineCompilerMode(compileRequest);

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

SlangResult SlangCTool::innerMain(StdWriters* stdWriters, slang::IGlobalSession* sharedSession, int argc, const char*const* argv)
{
    StdWriters::setSingleton(stdWriters);

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded stdlib.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(Index(argc - 1), argv + 1))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
    }

    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);

    // Do any app specific configuration
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        spSetWriter(compileRequest, SlangWriterChannel(i), stdWriters->getWriter(i));
    }

    SlangResult res = _compile(compileRequest, argc, argv);
    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);

    return res;
}
