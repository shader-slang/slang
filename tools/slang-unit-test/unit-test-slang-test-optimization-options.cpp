// unit-test-slang-test-optimization-options.cpp

#include "slang-test/slang-test-optimization-options.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(slangTestOptimizationArgDetection)
{
    const char* optimizationArgs[] = {
        "-O",
        "-O0",
        "-Onone",
        "-O1",
        "-Odefault",
        "-O2",
        "-Ohigh",
        "-O3",
        "-Omaximal",
    };

    for (const auto arg : optimizationArgs)
    {
        SLANG_CHECK(SlangTest::isSlangOptimizationArg(String(arg)));
    }

    SLANG_CHECK(!SlangTest::isSlangOptimizationArg(String("-Odump")));
    SLANG_CHECK(!SlangTest::isSlangOptimizationArg(String("-output")));

    {
        List<String> args;
        args.add("-target");
        args.add("spirv");

        SLANG_CHECK(!SlangTest::hasSlangOptimizationArg(args));

        args.add("-O2");

        SLANG_CHECK(SlangTest::hasSlangOptimizationArg(args));
    }
}

SLANG_UNIT_TEST(slangTestRenderOptimizationArgDetection)
{
    {
        List<String> args;
        args.add("-compile-arg");
        args.add("-O2");

        SLANG_CHECK(SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-xslang");
        args.add("-Ohigh");

        SLANG_CHECK(SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang");
        args.add("-O3");

        SLANG_CHECK(SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang...");
        args.add("-target");
        args.add("spirv");
        args.add("-Omaximal");
        args.add("-X.");

        SLANG_CHECK(SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang...");
        args.add("-target");
        args.add("spirv");
        args.add("-X.");
        args.add("-O3");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang...");
        args.add("-target");
        args.add("spirv");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-compile-arg");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-xslang");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }

    {
        List<String> args;
        args.add("-Xslang...");

        SLANG_CHECK(!SlangTest::hasRenderTestSlangOptimizationArg(args));
    }
}

SLANG_UNIT_TEST(slangTestDefaultOptimizationInsertion)
{
    {
        CommandLine cmdLine;

        SlangTest::addDefaultSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == SlangTest::kTestOptimizationOption);
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-O3");

        SlangTest::addDefaultSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == "-O3");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-vk");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-vk");
        SLANG_CHECK(cmdLine.m_args[1] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[2] == SlangTest::kTestOptimizationOption);
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-Xslang");
        cmdLine.addArg("-O3");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 2);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == "-O3");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-compile-arg");
        cmdLine.addArg("-O2");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 2);
        SLANG_CHECK(cmdLine.m_args[0] == "-compile-arg");
        SLANG_CHECK(cmdLine.m_args[1] == "-O2");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-xslang");
        cmdLine.addArg("-Ohigh");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 2);
        SLANG_CHECK(cmdLine.m_args[0] == "-xslang");
        SLANG_CHECK(cmdLine.m_args[1] == "-Ohigh");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-Xslang...");
        cmdLine.addArg("-target");
        cmdLine.addArg("spirv");
        cmdLine.addArg("-O3");
        cmdLine.addArg("-X.");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 5);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang...");
        SLANG_CHECK(cmdLine.m_args[1] == "-target");
        SLANG_CHECK(cmdLine.m_args[2] == "spirv");
        SLANG_CHECK(cmdLine.m_args[3] == "-O3");
        SLANG_CHECK(cmdLine.m_args[4] == "-X.");
    }

    // Metal render tests receive the Metal-specific default level; see
    // kMetalRenderTestOptimizationOption for why.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-mtl");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-mtl");
        SLANG_CHECK(cmdLine.m_args[1] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[2] == SlangTest::kMetalRenderTestOptimizationOption);
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-metal");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-metal");
        SLANG_CHECK(cmdLine.m_args[1] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[2] == SlangTest::kMetalRenderTestOptimizationOption);
    }

    // A Metal render test that already forwards a level keeps it.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-mtl");
        cmdLine.addArg("-Xslang");
        cmdLine.addArg("-O3");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine);

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-mtl");
        SLANG_CHECK(cmdLine.m_args[1] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[2] == "-O3");
    }
}
