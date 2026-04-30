#ifndef SLANG_MARKDOWN_H_INCLUDED
#define SLANG_MARKDOWN_H_INCLUDED

#include "../core/slang-basic.h"

namespace Slang
{

bool hasLiterateFileExtension(const String& path);

String maybeStripLiterateFileExtension(const String& path);

struct MarkdownCodeBlock
{
    String content;
    int startLine = 0;
    int startColumn = 0;
};

List<MarkdownCodeBlock> extractSlangCodeBlocks(const char* source, size_t length);

} // namespace Slang

#endif
