#include "test-output-path-util.h"

#include "core/slang-io.h"
#include "core/slang-string-escape-util.h"

namespace Slang
{

static void normalizeBareTestPath(const String& testDirectory, String& path)
{
    if (Path::hasPath(path))
        return;

    path = Path::combine(testDirectory, path);
}

// Returns true if `path` is absolute under any host's rules, including a Windows drive on a
// non-Windows host, so that a directive authored on one platform is rejected by every platform's
// CI.
static bool isAbsoluteOnAnyPlatform(const UnownedStringSlice& path)
{
    if (path.getLength() == 0)
        return false;

    // Leading `/` or `\`, which also covers a `\\server\share` UNC prefix.
    if (Path::isDelimiter(path[0]))
        return true;

    // A rooted Windows drive such as `C:\` or `C:/` (a bare `C:foo` is drive-relative, not caught).
    return Path::isDriveSpecification(Path::getFirstElement(path));
}

// Returns true if `path` is *this host's* null device, the sanctioned "discard the output" sink,
// which is exempt from the absolute-path rejection below (the POSIX spelling `/dev/null` is itself
// an absolute path). Matched host-specifically so the *other* host's spelling stays subject to the
// portability check: a `-o /dev/null` directive is still non-portable to Windows and is rejected
// there.
static bool isNullDeviceDiscardSink(const UnownedStringSlice& path)
{
#if SLANG_WINDOWS_FAMILY
    // Windows device names are case-insensitive.
    return path.caseInsensitiveEquals(UnownedStringSlice("NUL"));
#else
    // POSIX paths are case-sensitive, so `/DEV/NULL` is a different, non-null path.
    return path == UnownedStringSlice("/dev/null");
#endif
}

SlangResult normalizeTestOutputPathsForTestFile(
    const String& filePath,
    List<String>& args,
    String& outError)
{
    // Empty when the test path has no directory component; the absolute-path check below still runs
    // in that case, since the portability hazard does not depend on where the test file lives.
    String testDirectory = Path::getParentDirectory(filePath);

    for (Index i = 0; i < args.getCount(); ++i)
    {
        if ((args[i] != "-o" && args[i] != "-separate-debug-info-output") ||
            i + 1 >= args.getCount())
            continue;

        auto& outputPath = args[i + 1];

        // slang-test unescapes each argument shell-style before handing it to the compiler (see
        // `runCompile`), so classify the same unescaped form — otherwise a quoted `-o
        // "/tmp/out.spv"` would slip past the check on its leading `"`. `isUnescapeShellLikeNeeded`
        // returns non-zero when a quote is present, so it is used as a plain bool (not via
        // `SLANG_SUCCEEDED`).
        String unescapedPath = outputPath;
        StringEscapeHandler* escapeHandler =
            StringEscapeUtil::getHandler(StringEscapeUtil::Style::Space);
        if (StringEscapeUtil::isUnescapeShellLikeNeeded(
                escapeHandler,
                outputPath.getUnownedSlice()))
        {
            StringBuilder buf;
            // Malformed quoting (e.g. an unterminated `-o "/tmp/x`) produces an empty result that
            // would read as a non-absolute path and bypass the check below, so fail loudly instead.
            if (SLANG_FAILED(StringEscapeUtil::unescapeShellLike(
                    escapeHandler,
                    outputPath.getUnownedSlice(),
                    buf)))
            {
                StringBuilder builder;
                builder << "malformed quoting in path '" << outputPath << "' passed to '" << args[i]
                        << "'";
                outError = builder.produceString();
                return SLANG_E_INVALID_ARG;
            }
            unescapedPath = buf.produceString();
        }

        // Exempt from the absolute-path check: `-o -` (stdout) and this host's null-device discard
        // sink. Both are allowed output targets that need no rewriting.
        if (unescapedPath != "-" && !isNullDeviceDiscardSink(unescapedPath.getUnownedSlice()))
        {
            // An absolute output path cannot be reproduced across the platforms the suite runs on,
            // so reject it rather than silently rewrite it: there is no well-defined relative path
            // to rewrite it to.
            if (isAbsoluteOnAnyPlatform(unescapedPath.getUnownedSlice()))
            {
                StringBuilder builder;
                builder << "absolute path '" << outputPath << "' passed to '" << args[i]
                        << "' is not portable across platforms; use a relative path (a bare "
                           "filename is placed beside the test file), the host null device "
                           "('/dev/null' on POSIX, 'NUL' on Windows) to discard, or '"
                        << args[i] << " -' to write to stdout";
                outError = builder.produceString();
                return SLANG_E_INVALID_ARG;
            }

            if (testDirectory.getLength() != 0)
                normalizeBareTestPath(testDirectory, outputPath);
        }
        ++i;
    }

    return SLANG_OK;
}

} // namespace Slang
