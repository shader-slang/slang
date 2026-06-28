#ifndef TEST_OUTPUT_PATH_UTIL_H
#define TEST_OUTPUT_PATH_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{

// This helper rewrites bare `-o <file>` outputs and bare `-dump-intermediate-prefix <prefix>`
// values so test-owned artifacts stay beside `filePath`, while preserving path-qualified
// values and `-o -`. For example, `tests/a/b.slang` with `-o out.spv -dump-intermediates`
// becomes `-o tests/a/out.spv -dump-intermediate-prefix tests/a/b-`.
void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args);

} // namespace Slang

#endif
