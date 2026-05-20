// unit-test-test-tool-util.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-test-tool-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Verify that getExeDirectoryPath falls back to Path::getExecutablePath() for
// bare names (no separators), matching the fix in PR #11212.
SLANG_UNIT_TEST(testToolUtilBareArgv0)
{
    // A bare name with no path separator should use the OS-provided exe path.
    String bareResult;
    SLANG_CHECK(SLANG_SUCCEEDED(TestToolUtil::getExeDirectoryPath("slang-test", bareResult)));
    SLANG_CHECK(bareResult == Path::getParentDirectory(Path::getExecutablePath()));

    // Same for a Windows-style bare name.
    String bareExeResult;
    SLANG_CHECK(
        SLANG_SUCCEEDED(TestToolUtil::getExeDirectoryPath("slang-test.exe", bareExeResult)));
    SLANG_CHECK(bareExeResult == Path::getParentDirectory(Path::getExecutablePath()));
}
