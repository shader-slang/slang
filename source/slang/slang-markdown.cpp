#include "slang-markdown.h"

#include <cmark-gfm.h>
#include <string.h>

namespace Slang
{

List<MarkdownCodeBlock> extractSlangCodeBlocks(const char* source, size_t length)
{
    List<MarkdownCodeBlock> result;

    cmark_node* doc = cmark_parse_document(source, length, CMARK_OPT_SOURCEPOS);
    if (!doc)
        return result;

    for (cmark_node* node = cmark_node_first_child(doc); node; node = cmark_node_next(node))
    {
        if (cmark_node_get_type(node) != CMARK_NODE_CODE_BLOCK)
            continue;

        const char* fenceInfo = cmark_node_get_fence_info(node);
        if (fenceInfo && fenceInfo[0] != '\0' && strcmp(fenceInfo, "slang") != 0)
            continue;

        const char* literal = cmark_node_get_literal(node);
        if (!literal)
            continue;

        MarkdownCodeBlock block;
        block.content = literal;

        int fenceLength = 0;
        int fenceOffset = 0;
        char fenceChar = 0;
        bool isFenced = cmark_node_get_fenced(node, &fenceLength, &fenceOffset, &fenceChar) != 0;

        int nodeLine = cmark_node_get_start_line(node);
        block.startLine = isFenced ? nodeLine + 1 : nodeLine;

        block.startColumn = cmark_node_get_start_column(node);

        result.add(block);
    }

    cmark_node_free(doc);
    return result;
}

} // namespace Slang
