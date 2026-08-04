#ifndef TEST_OUTPUT_PATH_UTIL_H
#define TEST_OUTPUT_PATH_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{

// Rewrites every bare `-o <file>` and `-separate-debug-info-output <file>` value so test-owned
// artifacts stay beside `filePath`. Path-qualified values with either slash direction and `-o -`
// are preserved. For example, `tests/a/b.slang` with `-o out.spv` becomes
// `-o tests/a/out.spv`.
void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args);

} // namespace Slang

#endif
