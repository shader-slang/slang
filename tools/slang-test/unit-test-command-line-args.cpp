// unit-test-command-line-args.cpp

#include "../../source/compiler-core/slang-command-line-args.h"

#include "test-context.h"

using namespace Slang;

static void commandLineArgsUnitTest()
{
    RefPtr<CommandLineContext> context = new CommandLineContext;

    {
        CommandLineArgs args(context);
        DownstreamArgs downstreamArgs(context);

        DiagnosticSink sink(context->getSourceManager(), nullptr);
        
        const char* inArgs[] =
        {
            "-Xa...",
            "-blah",
            "10",
            "-X.",
        };

        args.setArgs(inArgs, SLANG_COUNT_OF(inArgs));

        SLANG_CHECK(SLANG_SUCCEEDED(downstreamArgs.stripDownstreamArgs(args, DownstreamArgs::Flag::AllowNewNames, &sink)));

        const char* aArgs[] =
        {
            "-blah",
            "10"
        };

        SLANG_CHECK(downstreamArgs.getArgs(downstreamArgs.findName("a")).hasArgs(aArgs, SLANG_COUNT_OF(aArgs)));
        SLANG_CHECK(args.getArgCount() == 0);
    }

}

SLANG_UNIT_TEST("CommandLineArgs", commandLineArgsUnitTest);
