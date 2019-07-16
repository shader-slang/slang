// slang-hex-dump-util.cpp
#include "slang-hex-dump-util.h"

#include "slang-common.h"
#include "slang-string-util.h"
#include "slang-writer.h"

#include "../../slang-com-helper.h"

namespace Slang
{

static const UnownedStringSlice s_start = UnownedStringSlice::fromLiteral("--START--");
static const UnownedStringSlice s_end = UnownedStringSlice::fromLiteral("--END--");

/* static */SlangResult HexDumpUtil::dumpWithMarkers(const List<uint8_t>& data, int maxBytesPerLine, ISlangWriter* writer)
{
    WriterHelper helper(writer);
    SLANG_RETURN_ON_FAIL(helper.write(s_start.begin(), s_start.size()));
    SLANG_RETURN_ON_FAIL(helper.print(" (%zu)\n", size_t(data.getCount())));

    SLANG_RETURN_ON_FAIL(dump(data, maxBytesPerLine, writer));

    SLANG_RETURN_ON_FAIL(helper.write(s_end.begin(), s_end.size()));
    SLANG_RETURN_ON_FAIL(helper.put("\n"));
    return SLANG_OK;
}

/* static */SlangResult HexDumpUtil::dump(const List<uint8_t>& data, int maxBytesPerLine, ISlangWriter* writer)
{
    int maxCharsPerLine = 2 * maxBytesPerLine + 1 + maxBytesPerLine + 1;

    const uint8_t* cur = data.begin();
    const uint8_t* end = data.end();

    static char s_hex[] = "0123456789abcdef";

    while (cur < end)
    {
        size_t count = size_t(end - cur);
        count = (count > size_t(maxBytesPerLine)) ? size_t(maxBytesPerLine) : count;

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

        SLANG_RETURN_ON_FAIL(writer->endAppendBuffer(startDst, size_t(dst - startDst)));

        cur += count;
    }

    return SLANG_OK;
}

static int _parseHexDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c -'0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

/* static */SlangResult HexDumpUtil::parse(const UnownedStringSlice& lines, List<uint8_t>& outBytes)
{
    outBytes.clear();

    bool inHex = false;

    LineParser lineParser(lines);
    for (const auto& line : lineParser)
    {
        if (!inHex)
        {
            if (line.startsWith(s_start))
            {
                inHex = true;
                continue;
            }
        }
        else
        {
            if (line.startsWith(s_end))
            {
                break;
            }
        }

        const char* cur = line.begin();
        const char* end = line.end();

        while(cur + 2 <= end)
        {
            const char c = cur[0];
            if (c == ' ' || c== '\n' || c == '\r' || c == '\t')
            {
                // Skip to next line
                break;
            }

            const int hi = _parseHexDigit(c);
            const int lo = _parseHexDigit(cur[1]);
            cur += 2;

            if (hi < 0 || lo < 0)
            {
                return SLANG_FAIL;
            }
            outBytes.add(uint8_t((hi << 4) | lo));
        }
    }

    return SLANG_OK;
}

/* static */SlangResult HexDumpUtil::parseWithMarkers(const UnownedStringSlice& lines, List<uint8_t>& outBytes)
{
    UnownedStringSlice remaining(lines), line;

    while(StringUtil::extractLine(remaining, line))
    {
        if (line.startsWith(s_start))
        {
            // Extract next line
            if (!StringUtil::extractLine(remaining, line))
            {
                return SLANG_FAIL;
            }
            // It's the start line
            UnownedStringSlice startLine = line;

            // Look for the ending line
            do 
            {
                if (line.startsWith(s_end))
                {
                    return parse(UnownedStringSlice(startLine.begin(), line.begin()), outBytes);
                }
            }
            while ( StringUtil::extractLine(remaining, line));
        }
    }

    return SLANG_FAIL;
}

}
