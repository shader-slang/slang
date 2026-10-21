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

enum class ApiFilterSelection
{
    Baseline,
    Filtered,
    SmokeCategory,
    SkipApiSteps,
    CompileSubtest,
    SynthesizedApiSteps,
};

// Exercise discovery, synthesis, and dispatch in the real runner. The child selects only
// temporary file tests, so it cannot recursively invoke this unit test. API steps are ignored
// or explicitly skipped; the mixed file's compile step must still execute alongside them.
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
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(apiOnly, "//TEST:COMPARE_COMPUTE: -cpu\n")));

    const String skipList = directory.getPath("skip.txt");
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllText(skipList, mixed + ".1\n" + mixed + ".2\n")));

    for (bool useServer : {false, true})
    {
        // The baseline proves both compile steps really run. The filtered cases must remove
        // only the independent file, including when subtest selection hides the API steps.
        for (auto selection : {
                 ApiFilterSelection::Baseline,
                 ApiFilterSelection::Filtered,
                 ApiFilterSelection::SmokeCategory,
                 ApiFilterSelection::SkipApiSteps,
                 ApiFilterSelection::CompileSubtest,
                 ApiFilterSelection::SynthesizedApiSteps,
             })
        {
            File::remove(independentOutput);
            File::remove(mixedOutput);

            CommandLine command;
            command.setExecutableLocation(
                ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
            command.addArg("-test-dir");
            command.addArg(directory.path);
            command.addArg("-skip-api-detection");
            command.addArg("-api");
            command.addArg(selection == ApiFilterSelection::SynthesizedApiSteps ? "cpu" : "none");
            command.addArg("-synthesizedTestApi");
            command.addArg(selection == ApiFilterSelection::SynthesizedApiSteps ? "cpu" : "none");
            if (useServer)
            {
                command.addArg("-use-test-server");
                command.addArg("-server-count");
                command.addArg("2");
            }
            if (selection != ApiFilterSelection::Baseline)
                command.addArg("-skip-api-independent-tests");
            if (selection == ApiFilterSelection::SmokeCategory)
            {
                command.addArg("-category");
                command.addArg("smoke");
            }
            if (selection == ApiFilterSelection::SkipApiSteps ||
                selection == ApiFilterSelection::SynthesizedApiSteps)
            {
                command.addArg("-skip-list");
                command.addArg(skipList);
            }
            command.addArg(
                selection == ApiFilterSelection::CompileSubtest
                    ? mixed + ".0"
                    : (selection == ApiFilterSelection::SynthesizedApiSteps ? mixed
                                                                            : directory.path));
            if (selection == ApiFilterSelection::SynthesizedApiSteps)
                command.addArg("-v");

            ExecuteResult result;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(ProcessUtil::execute(command, result)));
            const bool independentMatches =
                File::exists(independentOutput) == (selection == ApiFilterSelection::Baseline);
            const bool mixedCompiled = File::exists(mixedOutput);

            // Synthesis is limited to enabled APIs. Enable CPU only in this case, then skip
            // both dispatches explicitly. The verbose skip message proves the variant exists
            // while the output file proves the compile step still runs alongside it.
            const String expected = "skipping excluded subtest: " + mixed + ".2 syn (cpu)";
            const bool synthesizedSkipped = selection != ApiFilterSelection::SynthesizedApiSteps ||
                                            result.standardOutput.indexOf(expected) >= 0 ||
                                            result.standardError.indexOf(expected) >= 0;
            if (result.resultCode != 0 || !independentMatches || !mixedCompiled ||
                !synthesizedSkipped)
                printf(
                    "%s\n%s\n",
                    result.standardOutput.getBuffer(),
                    result.standardError.getBuffer());
            SLANG_CHECK(result.resultCode == 0);
            SLANG_CHECK(independentMatches);
            SLANG_CHECK(mixedCompiled);
            SLANG_CHECK(synthesizedSkipped);
        }
    }
}
} // namespace
