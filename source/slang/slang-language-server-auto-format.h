#pragma once

#include "../../slang.h"
#include "../core/slang-basic.h"
#include "slang-workspace-version.h"

namespace Slang
{
struct Edit
{
    Index offset;
    Index length;
    String text;
};

struct FormatOptions
{
    String clangFormatLocation;
    String style = "{BasedOnStyle: Microsoft}";
};

String findClangFormatTool();
List<Edit> formatSource(UnownedStringSlice text, Index lineStart, Index lineEnd, Index cursorOffset, const FormatOptions& options);

} // namespace Slang
