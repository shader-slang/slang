#include "test-output-path-util.h"

#include "core/slang-io.h"

namespace Slang
{

static bool hasArg(const List<String>& args, const char* option)
{
    for (const auto& arg : args)
    {
        if (arg == option)
            return true;
    }
    return false;
}

void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args)
{
    String testDirectory = Path::getParentDirectory(filePath);
    if (testDirectory.getLength() == 0)
        return;

    for (Index i = 0; i + 1 < args.getCount(); ++i)
    {
        if (args[i] != "-o")
            continue;

        auto& outputPath = args[i + 1];
        if (outputPath != "-" && !Path::hasPath(outputPath))
            outputPath = Path::combine(testDirectory, outputPath);

        ++i;
    }

    if (hasArg(args, "-dump-intermediates") && !hasArg(args, "-dump-intermediate-prefix"))
    {
        String dumpPrefix =
            Path::combine(testDirectory, Path::getFileNameWithoutExt(filePath) + String("-"));
        args.add("-dump-intermediate-prefix");
        args.add(dumpPrefix);
    }
}

} // namespace Slang
