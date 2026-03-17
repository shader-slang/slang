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
    int startLine;
    int startColumn;
};

List<MarkdownCodeBlock> extractSlangCodeBlocks(const char* source, size_t length);

} // namespace Slang

#endif
