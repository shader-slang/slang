// slang-hex-dump-util.cpp
#include "slang-hex-dump-util.h"

#include "slang-common.h"

namespace Slang
{

/* static */SlangResult HexDumpUtil::dump(const List<uint8_t>& data, int maxBytesPerLine, ISlangWriter* writer)
{
    int maxCharsPerLine = 2 * maxBytesPerLine + 1 + maxBytesPerLine + 1;

    const uint8_t* cur = data.begin();
    const uint8_t* end = data.end();

    static char s_hex[] = "0123456789abcdef";

    while (cur < end)
    {
        size_t count = size_t(end - cur);
        count = (count > maxBytesPerLine) ? maxBytesPerLine : count;

        char* startDst = writer->beginAppendBuffer(maxCharsPerLine);
        char* dst = startDst;

        for (size_t i = 0; i < count; ++i)
        {
            uint8_t byte = cur[i];
            *dst++ = s_hex[byte >> 4];
            *dst++ = s_hex[byte & 0xf];
        }

        *dst++ = ' ';

        for (size_t i = 0; i < count; ++i)
        {
            char c = char(cur[i]);

            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= 32 && (c & 0x80) == 0))
            {
            }
            else
            {
                c = '.';
            }
            *dst++ = c;
        }

        *dst++ = '\n';
        SLANG_ASSERT(dst <= startDst + maxCharsPerLine);

        writer->endAppendBuffer(startDst, size_t(dst - startDst));

        cur += count;
    }
}



}
