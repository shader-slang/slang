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

    const char* testOptimizationArgs[] = {"-O0", "-O1", "-O2", "-O3"};
    for (const auto arg : testOptimizationArgs)
    {
        SLANG_CHECK(SlangTest::isSlangTestOptimizationArg(UnownedStringSlice(arg)));
    }

    SLANG_CHECK(!SlangTest::isSlangTestOptimizationArg(UnownedStringSlice("-O")));
    SLANG_CHECK(!SlangTest::isSlangTestOptimizationArg(UnownedStringSlice("-O4")));
    SLANG_CHECK(!SlangTest::isSlangTestOptimizationArg(UnownedStringSlice("-Ohigh")));

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

        SlangTest::addDefaultSlangOptimization(cmdLine, String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == SlangTest::kTestOptimizationOption);
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-O3");

        SlangTest::addDefaultSlangOptimization(cmdLine, String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == "-O3");
    }

    // The default is inserted at the front so it cannot be consumed as the argument of a
    // trailing option that a diagnostic test intentionally leaves dangling.
    {
        CommandLine cmdLine;
        cmdLine.addArg("test.slang");
        cmdLine.addArg("-profile");
        cmdLine.addArg("ps_4_0");
        cmdLine.addArg("-target");

        SlangTest::addDefaultSlangOptimization(cmdLine, String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 5);
        SLANG_CHECK(cmdLine.m_args[0] == SlangTest::kTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[4] == "-target");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-vk");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == SlangTest::kTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[2] == "-vk");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-Xslang");
        cmdLine.addArg("-O3");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 2);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == "-O3");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-compile-arg");
        cmdLine.addArg("-O2");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 2);
        SLANG_CHECK(cmdLine.m_args[0] == "-compile-arg");
        SLANG_CHECK(cmdLine.m_args[1] == "-O2");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-xslang");
        cmdLine.addArg("-Ohigh");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

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

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

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

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == SlangTest::kMetalRenderTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[2] == "-mtl");
    }

    {
        CommandLine cmdLine;
        cmdLine.addArg("-metal");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == SlangTest::kMetalRenderTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[2] == "-metal");
    }

    // A Metal render test that already forwards a level keeps it.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-mtl");
        cmdLine.addArg("-Xslang");
        cmdLine.addArg("-O3");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-mtl");
        SLANG_CHECK(cmdLine.m_args[1] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[2] == "-O3");
    }

    // The render-test default is inserted at the front, so a directive that accidentally
    // leaves a forwarding flag without its value cannot consume the inserted default; the
    // mistake stays visible as a dangling trailing flag.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-vk");
        cmdLine.addArg("-xslang");

        SlangTest::addDefaultRenderTestSlangOptimization(
            cmdLine,
            String(SlangTest::kTestOptimizationOption));

        SLANG_CHECK(cmdLine.m_args.getCount() == 4);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == SlangTest::kTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[2] == "-vk");
        SLANG_CHECK(cmdLine.m_args[3] == "-xslang");
    }
}

SLANG_UNIT_TEST(slangTestDefaultOptimizationOverride)
{
    // A compiler test with no pinned level takes the command-line override.
    {
        CommandLine cmdLine;

        SlangTest::addDefaultSlangOptimization(cmdLine, String("-O2"));

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == "-O2");
    }

    // An optimization level in the test directive remains the source of truth.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-O1");

        SlangTest::addDefaultSlangOptimization(cmdLine, String("-O2"));

        SLANG_CHECK(cmdLine.m_args.getCount() == 1);
        SLANG_CHECK(cmdLine.m_args[0] == "-O1");
    }

    // Render-test must forward the override to slangc.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-vk");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine, String("-O2"));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == "-O2");
        SLANG_CHECK(cmdLine.m_args[2] == "-vk");
    }

    // Metal render tests keep the stable downstream-toolchain level.
    {
        CommandLine cmdLine;
        cmdLine.addArg("-metal");

        SlangTest::addDefaultRenderTestSlangOptimization(cmdLine, String("-O2"));

        SLANG_CHECK(cmdLine.m_args.getCount() == 3);
        SLANG_CHECK(cmdLine.m_args[0] == "-Xslang");
        SLANG_CHECK(cmdLine.m_args[1] == SlangTest::kMetalRenderTestOptimizationOption);
        SLANG_CHECK(cmdLine.m_args[2] == "-metal");
    }
}
