#ifndef TEST_OUTPUT_PATH_UTIL_H
#define TEST_OUTPUT_PATH_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang.h"

namespace Slang
{

// Rewrites every bare `-o <file>` and `-separate-debug-info-output <file>` value so test-owned
// artifacts stay beside `filePath` (e.g. `tests/a/b.slang` with `-o out.spv` becomes
// `-o tests/a/out.spv`), and validates the value for cross-platform portability.
//
// A value is rejected (returns `SLANG_E_INVALID_ARG`, with `outError` naming the option, the path,
// and the allowed alternatives) when it is an absolute path — a POSIX `/tmp/...` is meaningless
// on Windows and a `C:\...` on POSIX. The absolute check is platform-independent, so a
// Windows-shaped path is caught on Linux CI and vice versa. Malformed quoting (an unterminated
// quote) is also rejected rather than silently unescaped to an empty string.
//
// These output targets are exempt and preserved unchanged: `-o -` (write to stdout) and the host's
// null-device discard sink (`/dev/null` on POSIX, `NUL` on Windows — only the host's own spelling
// is recognised). Path-qualified relative values are also preserved.
SlangResult normalizeTestOutputPathsForTestFile(
    const String& filePath,
    List<String>& args,
    String& outError);

} // namespace Slang

#endif
