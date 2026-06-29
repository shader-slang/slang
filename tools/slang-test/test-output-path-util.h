#ifndef TEST_OUTPUT_PATH_UTIL_H
#define TEST_OUTPUT_PATH_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"

namespace Slang
{

// Rewrites every bare `-o <file>` output and bare `-dump-intermediate-prefix <prefix>` value so
// test-owned artifacts stay beside `filePath`. Path-qualified values with either slash direction
// and `-o -` are preserved. If `-dump-intermediates` appears without an explicit prefix, the helper
// adds one beside `filePath`. For example, `tests/a/b.slang` with `-o out.spv
// -dump-intermediates` becomes `-o tests/a/out.spv -dump-intermediate-prefix tests/a/b-`.
void normalizeTestOutputPathsForTestFile(const String& filePath, List<String>& args);

} // namespace Slang

#endif
