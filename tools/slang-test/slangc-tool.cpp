// test-context.cpp
#include "slangc-tool.h"

#include "../../source/core/slang-exception.h"
#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-io.h"

using namespace Slang;

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
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

    ComPtr<slang::ICompileRequest> compileRequest;
    SLANG_RETURN_ON_FAIL(session->createCompileRequest(compileRequest.writeRef()));

    auto compilerExecutablePath = Path::getParentDirectory(Path::getExecutablePath());
    compileRequest->addSearchPath(compilerExecutablePath.getBuffer());

    // Do any app specific configuration
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        const auto channel = SlangWriterChannel(i);
        compileRequest->setWriter(channel, stdWriters->getWriter(channel));
    }

    compileRequest->setDiagnosticCallback(&_diagnosticCallback, nullptr);
    compileRequest->setCommandLineCompilerMode();

    {
        const SlangResult res = compileRequest->processCommandLineArguments(&argv[1], argc - 1);
        if (SLANG_FAILED(res))
        {
            // TODO: print usage message
            return res;
        }
    }

    SlangResult compileRes = SLANG_OK;

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        compileRes = compileRequest->compile();

        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match previous behavior
        compileRes = SLANG_FAILED(compileRes) ? SLANG_E_INTERNAL_FAIL : compileRes;
    }
#ifndef _DEBUG
    catch (const Exception& e)
    {
        StdWriters::getOut().print("internal compiler error: %S\n", e.Message.toWString().begin());
        compileRes = SLANG_FAIL;
    }
#endif

    return compileRes;
}
