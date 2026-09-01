// unit-test-metal-compile-args.cpp

#include "compiler-core/slang-gcc-compiler-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Neither flag is observable in the emitted MSL or in the metallib -- one built without
// `-fmetal-enable-logging` still compiles, it just drops `os_log` calls at runtime -- so the
// command line is the only level at which losing either is caught.
SLANG_UNIT_TEST(metalCompileArgs)
{
    const auto hasArg = [](const DownstreamCompileOptions& options, const char* arg)
    {
        CommandLine cmdLine;
        SLANG_CHECK(SLANG_SUCCEEDED(GCCDownstreamCompilerUtil::calcArgs(options, cmdLine)));
        return cmdLine.findArgIndex(UnownedStringSlice(arg)) >= 0;
    };

    DownstreamCompileOptions baseOptions;
    baseOptions.targetType = SLANG_METAL_LIB;
    baseOptions.modulePath = TerminatedCharSlice("test-module");

    SLANG_CHECK(hasArg(baseOptions, "-std=metal3.1"));
    SLANG_CHECK(!hasArg(baseOptions, "-fmetal-enable-logging"));

    {
        DownstreamCompileOptions options = baseOptions;
        options.metalLanguageVersion = SemanticVersion(3, 2);
        options.flags |= DownstreamCompileOptions::Flag::EnableLogging;

        SLANG_CHECK(hasArg(options, "-std=metal3.2"));
        SLANG_CHECK(!hasArg(options, "-std=metal3.1"));
        SLANG_CHECK(hasArg(options, "-fmetal-enable-logging"));
    }

    {
        DownstreamCompileOptions options = baseOptions;
        options.targetType = SLANG_SHADER_SHARED_LIBRARY;
        options.metalLanguageVersion = SemanticVersion(3, 2);
        options.flags |= DownstreamCompileOptions::Flag::EnableLogging;

        SLANG_CHECK(!hasArg(options, "-std=metal3.2"));
        SLANG_CHECK(!hasArg(options, "-fmetal-enable-logging"));
    }
}
