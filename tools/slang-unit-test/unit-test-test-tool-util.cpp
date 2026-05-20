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

    // nullptr argv[0] — exercises the new null guard in _getCanonicalOrExecutablePath.
    String nullResult;
    SLANG_CHECK(SLANG_SUCCEEDED(TestToolUtil::getExeDirectoryPath(nullptr, nullResult)));
    SLANG_CHECK(nullResult == Path::getParentDirectory(Path::getExecutablePath()));

    // Empty string — hasPath("") == false, must fall through to OS fallback.
    String emptyResult;
    SLANG_CHECK(SLANG_SUCCEEDED(TestToolUtil::getExeDirectoryPath("", emptyResult)));
    SLANG_CHECK(emptyResult == Path::getParentDirectory(Path::getExecutablePath()));

    // Path-like but unresolvable — hasPath() == true, getCanonical fails, must fall through.
    String missingResult;
    SLANG_CHECK(SLANG_SUCCEEDED(
        TestToolUtil::getExeDirectoryPath("/nonexistent/dir/slang-test", missingResult)));
    SLANG_CHECK(missingResult == Path::getParentDirectory(Path::getExecutablePath()));
}
