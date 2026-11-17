// slang-reflection-test-main.cpp

#include "compiler-core/slang-pretty-writer.h"
#include "core/slang-char-util.h"
#include "core/slang-string-escape-util.h"
#include "core/slang-string-util.h"
#include "core/slang-test-tool-util.h"
#include "slang-com-helper.h"
#include "slang.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

void emitReflectionJSON(SlangCompileRequest* request, SlangReflection* reflection)
{
    ComPtr<ISlangBlob> b;

    spReflection_ToJson(reflection, request, b.writeRef());

    // Output the writer content to out stream
    StdWriters::getOut().write((const char*)b->getBufferPointer(), b->getBufferSize());
}

static SlangResult maybeDumpDiagnostic(SlangResult res, SlangCompileRequest* request)
{
    const char* diagnostic;
    if (SLANG_FAILED(res) && (diagnostic = spGetDiagnosticOutput(request)))
    {
        StdWriters::getError().put(diagnostic);
    }
    return res;
}

SlangResult performCompilationAndReflection(
    SlangCompileRequest* request,
    int argc,
    const char* const* argv)
{
    SLANG_RETURN_ON_FAIL(
        maybeDumpDiagnostic(spProcessCommandLineArguments(request, &argv[1], argc - 1), request));

    // A failed compilation returns `SLANG_E_INTERNAL_FAIL`, so that the exit code of this tool is
    // -1 as it is for `slangc` (see `TestToolUtil::getReturnCode`). An invalid command line
    // returns `SLANG_FAIL` above, and the exit code is 1. `SLANG_E_ABORT` stays as it is, because
    // it reports an internal compiler error, which has its own exit code.
    SlangResult compileResult = maybeDumpDiagnostic(spCompile(request), request);
    if (SLANG_FAILED(compileResult))
        return compileResult == SLANG_E_ABORT ? compileResult : SLANG_E_INTERNAL_FAIL;

    // Okay, let's go through and emit reflection info on whatever
    // we have.

    SlangReflection* reflection = spGetReflection(request);
    emitReflectionJSON(request, reflection);

    return SLANG_OK;
}

SLANG_TEST_TOOL_API SlangResult
innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char* const* argv)
{
    Slang::StdWriters::setSingleton(stdWriters);

    SlangCompileRequest* request = spCreateCompileRequest(session);
    for (int i = 0; i < SLANG_WRITER_CHANNEL_COUNT_OF; ++i)
    {
        const auto channel = SlangWriterChannel(i);
        spSetWriter(request, channel, stdWriters->getWriter(channel));
    }

    SlangResult res = performCompilationAndReflection(request, argc, argv);

    spDestroyCompileRequest(request);

    return res;
}

int main(int argc, char** argv)
{
    using namespace Slang;

    SlangSession* session = spCreateSession(nullptr);

    auto stdWriters = StdWriters::initDefaultSingleton();

    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);
    slang::shutdown();
    return (int)TestToolUtil::getReturnCode(res);
}
