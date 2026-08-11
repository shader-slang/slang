#include "test-output-path-util.h"

#include "core/slang-io.h"

namespace Slang
{

static void normalizeBareTestPath(const String& testDirectory, String& path)
{
    if (Path::hasPath(path))
        return;

    path = Path::combine(testDirectory, path);
}

void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args)
{
    String testDirectory = Path::getParentDirectory(filePath);
    if (testDirectory.getLength() == 0)
        return;

    for (Index i = 0; i < args.getCount(); ++i)
    {
        if ((args[i] != "-o" && args[i] != "-separate-debug-info-output") ||
            i + 1 >= args.getCount())
            continue;

        auto& outputPath = args[i + 1];
        if (outputPath != "-")
            normalizeBareTestPath(testDirectory, outputPath);
        ++i;
    }
}

} // namespace Slang
