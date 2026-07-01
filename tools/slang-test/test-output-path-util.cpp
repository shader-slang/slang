#include "test-output-path-util.h"

#include "core/slang-io.h"

namespace Slang
{

String getTestRelativePath(const String& filePath, const String& path)
{
    if (Path::hasPath(path))
        return path;

    String testDirectory = Path::getParentDirectory(filePath);
    if (testDirectory.getLength() == 0)
        return path;

    return Path::combine(testDirectory, path);
}

void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args)
{
    String testDirectory = Path::getParentDirectory(filePath);
    if (testDirectory.getLength() == 0)
        return;

    bool hasDumpIntermediates = false;
    bool hasDumpIntermediatePrefix = false;

    for (Index i = 0; i < args.getCount(); ++i)
    {
        if (args[i] == "-dump-intermediates")
        {
            hasDumpIntermediates = true;
            continue;
        }

        if (args[i] == "-dump-intermediate-prefix")
        {
            hasDumpIntermediatePrefix = true;
            if (i + 1 < args.getCount())
            {
                args[i + 1] = getTestRelativePath(filePath, args[i + 1]);
                ++i;
            }
            continue;
        }

        if (args[i] != "-o" || i + 1 >= args.getCount())
            continue;

        auto& outputPath = args[i + 1];
        if (outputPath != "-")
            outputPath = getTestRelativePath(filePath, outputPath);
        ++i;
    }

    if (hasDumpIntermediates && !hasDumpIntermediatePrefix)
    {
        String dumpPrefix =
            Path::combine(testDirectory, Path::getFileNameWithoutExt(filePath) + String("-"));
        args.add("-dump-intermediate-prefix");
        args.add(dumpPrefix);
    }
}

} // namespace Slang
