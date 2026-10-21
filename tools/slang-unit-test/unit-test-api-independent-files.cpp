#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
struct ApiFilterTestDirectory
{
    String path;

    // Match the forward-slash paths used by test discovery and positional prefixes.
    String getPath(const char* name) const { return Path::simplify(Path::combine(path, name)); }

    ~ApiFilterTestDirectory()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

// Exercise discovery, synthesis, and dispatch in the real runner. The child selects only
// temporary file tests, so it cannot recursively invoke this unit test. API steps are ignored
// or explicitly skipped in the mixed-file cases; its compile step must still execute.
// The API-only cases dispatch on CPU and check its result, so ignoring that file cannot pass.
SLANG_UNIT_TEST(slangTestSkipApiIndependentFiles)
{
    ApiFilterTestDirectory directory;
    String temporaryFile;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::generateTemporary(UnownedStringSlice("slang-api-filter"), temporaryFile)));
    File::remove(temporaryFile);
    // Positional test prefixes are normalized without their root, so keep the fixture paths
    // relative to the working directory too. The temporary filename supplies a unique name.
    directory.path = Path::getFileName(temporaryFile) + ".d";
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(directory.path));

    const String source = directory.getPath("source.slang");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        source,
        "[shader(\"compute\")]\n[numthreads(1, 1, 1)]\nvoid main() {}\n")));

    const String independent = directory.getPath("independent.slang");
    const String mixed = directory.getPath("mixed.slang");
    const String independentOutput = directory.getPath("independent.hlsl");
    const String mixedOutput = directory.getPath("mixed.hlsl");
    StringBuilder independentText;
    independentText << "//TEST(smoke):COMPILE: \"" << source << "\" -target hlsl -entry main -o \""
                    << independentOutput << "\"\n";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(independent, independentText)));

    StringBuilder mixedText;
    mixedText << "//TEST(smoke):COMPILE: \"" << source << "\" -target hlsl -entry main -o \""
              << mixedOutput << "\"\n"
              << "//TEST:COMPARE_COMPUTE:\n";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(mixed, mixedText)));

    const String apiOnly = directory.getPath("api-only.slang");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
        apiOnly,
        "//TEST:COMPARE_COMPUTE: -cpu\n"
        "//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer\n"
        "RWStructuredBuffer<uint> outputBuffer;\n"
        "[numthreads(1, 1, 1)]\n"
        "void computeMain(uint3 id : SV_DispatchThreadID) { outputBuffer[0] = 1; }\n")));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(apiOnly + ".expected.txt", "1\n")));

    const String skipList = directory.getPath("skip.txt");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(skipList, mixed + ".1\n" + mixed + ".2\n")));

    struct Scenario
    {
        const char* name;
        List<String> arguments;
        String prefix;
        bool expectIndependentOutput;
        bool expectMixedOutput;
        String expectedReport;
    };

    const String apiPassed = "passed test: '" + apiOnly + " (cpu)'";
    const String synthesizedSkipped = "skipping excluded subtest: " + mixed + ".2 syn (cpu)";
    const Scenario scenarios[] = {
        {"baseline",
         {"-api", "none", "-synthesizedTestApi", "none"},
         directory.path,
         true,
         true,
         ""},
        {"filtered",
         {"-api", "none", "-synthesizedTestApi", "none", "-skip-api-independent-tests"},
         directory.path,
         false,
         true,
         ""},
        {"smoke category",
         {"-api",
          "none",
          "-synthesizedTestApi",
          "none",
          "-skip-api-independent-tests",
          "-category",
          "smoke"},
         directory.path,
         false,
         true,
         ""},
        {"API steps skipped",
         {"-api",
          "none",
          "-synthesizedTestApi",
          "none",
          "-skip-api-independent-tests",
          "-skip-list",
          skipList},
         directory.path,
         false,
         true,
         ""},
        {"compile subtest",
         {"-api", "none", "-synthesizedTestApi", "none", "-skip-api-independent-tests"},
         mixed + ".0",
         false,
         true,
         ""},
        {"synthesized API step skipped",
         {"-api",
          "cpu",
          "-synthesizedTestApi",
          "cpu",
          "-skip-api-independent-tests",
          "-skip-list",
          skipList},
         mixed,
         false,
         true,
         synthesizedSkipped},
        {"API-only baseline",
         {"-api", "cpu", "-synthesizedTestApi", "none"},
         apiOnly,
         false,
         false,
         apiPassed},
        {"API-only filtered",
         {"-api", "cpu", "-synthesizedTestApi", "none", "-skip-api-independent-tests"},
         apiOnly,
         false,
         false,
         apiPassed},
    };

    for (bool useServer : {false, true})
    {
        for (const auto& scenario : scenarios)
        {
            File::remove(independentOutput);
            File::remove(mixedOutput);

            CommandLine command;
            command.setExecutableLocation(
                ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
            command.addArg("-test-dir");
            command.addArg(directory.path);
            command.addArg("-skip-api-detection");
            command.addArg("-disable-retries");
            command.addArg("-v");
            for (const auto& argument : scenario.arguments)
                command.addArg(argument);
            if (useServer)
            {
                command.addArg("-use-test-server");
                command.addArg("-server-count");
                command.addArg("2");
            }
            command.addArg(scenario.prefix);

            ExecuteResult result;
            const SlangResult executeResult = ProcessUtil::execute(command, result);
            const bool independentMatches =
                File::exists(independentOutput) == scenario.expectIndependentOutput;
            const bool mixedMatches = File::exists(mixedOutput) == scenario.expectMixedOutput;
            const bool reportMatches =
                scenario.expectedReport.getLength() == 0 ||
                result.standardOutput.indexOf(scenario.expectedReport) >= 0 ||
                result.standardError.indexOf(scenario.expectedReport) >= 0;
            if (SLANG_FAILED(executeResult) || result.resultCode != 0 || !independentMatches ||
                !mixedMatches || !reportMatches)
            {
                printf(
                    "Scenario '%s' (%s): launch=0x%08x, exit=%d\n%s\n%s\n",
                    scenario.name,
                    useServer ? "test-server" : "direct",
                    unsigned(executeResult),
                    result.resultCode,
                    result.standardOutput.getBuffer(),
                    result.standardError.getBuffer());
            }
            SLANG_CHECK(SLANG_SUCCEEDED(executeResult));
            SLANG_CHECK(result.resultCode == 0);
            SLANG_CHECK(independentMatches);
            SLANG_CHECK(mixedMatches);
            SLANG_CHECK(reportMatches);
        }
    }
}
} // namespace
