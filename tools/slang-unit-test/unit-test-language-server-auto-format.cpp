// unit-test-language-server-auto-format.cpp

#include "slang/slang-language-server-auto-format.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(languageServerClangFormatLocation)
{
    String executableName = String("clang-format") + String(Process::getExecutableSuffix());

    SLANG_CHECK(isClangFormatExecutableName(executableName.getUnownedSlice()));
    SLANG_CHECK(isSafeClangFormatLocation(executableName.getUnownedSlice()));
    SLANG_CHECK(!isSafeClangFormatExecutablePath(executableName.getUnownedSlice()));

#if SLANG_WINDOWS_FAMILY
    SLANG_CHECK(isClangFormatExecutableName(toSlice("ClAnG-FoRmAt.ExE")));
    SLANG_CHECK(isSafeClangFormatLocation(toSlice("ClAnG-FoRmAt.ExE")));
#else
    SLANG_CHECK(!isClangFormatExecutableName(toSlice("ClAnG-FoRmAt")));
    SLANG_CHECK(!isSafeClangFormatLocation(toSlice("ClAnG-FoRmAt")));
#endif

    SLANG_CHECK(!isSafeClangFormatLocation(toSlice("")));
    SLANG_CHECK(!isSafeClangFormatLocation(toSlice("not-clang-format")));
    SLANG_CHECK(!isSafeClangFormatLocation(toSlice("clang-format-malicious")));
    SLANG_CHECK(!isSafeClangFormatLocation(
        (String("${workspaceFolder}/") + executableName).getUnownedSlice()));
    SLANG_CHECK(!isSafeClangFormatLocation(
        (String("/workspace/tools/") + executableName).getUnownedSlice()));

#if SLANG_WINDOWS_FAMILY
    String absoluteLocation = String("C:/tools/") + executableName;
#else
    String absoluteLocation = String("/tools/") + executableName;
#endif
    SLANG_CHECK(isSafeClangFormatExecutablePath(absoluteLocation.getUnownedSlice()));
}
