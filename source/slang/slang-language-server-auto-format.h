#pragma once

#include "core/slang-basic.h"
#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-workspace-version.h"
#include "slang.h"

namespace Slang
{
struct Edit
{
    Index offset;
    Index length;
    String text;
};

struct TextRange
{
    Index offsetStart;
    Index offsetEnd;
};

enum class FormatBehavior
{
    Standard,
    PreserveLineBreak,
};

struct FormatOptions
{
    bool enableFormatOnType = true;
    String clangFormatLocation;
    String style = "file";
    String fallbackStyle = "{BasedOnStyle: Microsoft}";
    String fileName;
    bool allowLineBreakInOnTypeFormatting = false;
    bool allowLineBreakInRangeFormatting = false;
    FormatBehavior behavior = FormatBehavior::Standard;
};

/// Returns whether `name` is the canonical clang-format executable name for this platform.
inline bool isClangFormatExecutableName(UnownedStringSlice name)
{
    String expectedName = String("clang-format") + String(Process::getExecutableSuffix());
#if SLANG_WINDOWS_FAMILY
    return name.caseInsensitiveEquals(expectedName.getUnownedSlice());
#else
    return name == expectedName.getUnownedSlice();
#endif
}

/// Returns whether an LSP configuration value can safely select clang-format through `PATH`.
///
/// Paths are intentionally rejected because workspace configuration is controlled by the opened
/// project. Allowing a path here would let that project select its own executable.
inline bool isSafeClangFormatLocation(UnownedStringSlice location)
{
    return !Path::hasPath(location) && isClangFormatExecutableName(location);
}

/// Returns whether a resolved path is safe to pass to process creation as clang-format.
inline bool isSafeClangFormatExecutablePath(UnownedStringSlice path)
{
    return Path::isAbsolute(path) &&
           isClangFormatExecutableName(Path::getFileName(path).getUnownedSlice());
}

String findClangFormatTool();

List<TextRange> extractFormattingExclusionRanges(UnownedStringSlice text);

List<Edit> formatSource(
    UnownedStringSlice text,
    Index lineStart,
    Index lineEnd,
    Index cursorOffset,
    const List<TextRange>& exclusionRanges,
    const FormatOptions& options);

} // namespace Slang
