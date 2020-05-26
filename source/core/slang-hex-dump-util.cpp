// slang-hex-dump-util.cpp
#include "slang-hex-dump-util.h"

#include "slang-common.h"
#include "slang-string-util.h"
#include "slang-writer.h"

#include "../../slang-com-helper.h"
#include "slang-hash.h"

namespace Slang
{

static const UnownedStringSlice s_start = UnownedStringSlice::fromLiteral("--START--");
static const UnownedStringSlice s_end = UnownedStringSlice::fromLiteral("--END--");
static const char s_hex[] = "0123456789abcdef";

/* static */SlangResult HexDumpUtil::dumpWithMarkers(const List<uint8_t>& data, int maxBytesPerLine, ISlangWriter* writer)
{
    return dumpWithMarkers(data.getBuffer(), data.getCount(), maxBytesPerLine, writer);
}

/* static */SlangResult HexDumpUtil::dumpWithMarkers(const uint8_t* data, size_t dataCount, int maxBytesPerLine, ISlangWriter* writer)
{
    WriterHelper helper(writer);
    SLANG_RETURN_ON_FAIL(helper.write(s_start.begin(), s_start.getLength()));
    SLANG_RETURN_ON_FAIL(helper.print(" %zu", dataCount));

    const HashCode32 hash = getStableHashCode32((const char*)data, dataCount);
    SLANG_RETURN_ON_FAIL(helper.print(" %d\n", int(hash) ));

    SLANG_RETURN_ON_FAIL(dump(data, dataCount, maxBytesPerLine, writer));

    SLANG_RETURN_ON_FAIL(helper.write(s_end.begin(), s_end.getLength()));
    SLANG_RETURN_ON_FAIL(helper.put("\n"));
    return SLANG_OK;
}

/* static */void HexDumpUtil::dump(uint32_t value, ISlangWriter* writer)
{
    char c[9];
    for (int i = 0; i < 8; ++i)
    {
        c[i] = s_hex[value >> 28];
        value <<= 4;
    }
    writer->write(c, 8);
}


/* static */SlangResult HexDumpUtil::dump(const List<uint8_t>& data, int maxBytesPerLine, ISlangWriter* writer)
{
    return dump(data.getBuffer(), data.getCount(), maxBytesPerLine, writer);
}

/* static */SlangResult HexDumpUtil::dump(const uint8_t* data, size_t dataCount, int maxBytesPerLine, ISlangWriter* writer)
{
    int maxCharsPerLine = 2 * maxBytesPerLine + 1 + maxBytesPerLine + 1;

    const uint8_t* cur = data;
    const uint8_t* end = data + dataCount;
    
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

        // If not a complete line write spaces
        for (size_t i = count; i < size_t(maxBytesPerLine); ++i)
        {
            *dst++ = ' ';
            *dst++ = ' '; 
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

    LineParser lineParser(lines);
    for (const auto& line : lineParser)
    {
        const char* cur = line.begin();
        const char* end = line.end();

        while(cur + 2 <= end)
        {
            const char c = cur[0];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
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

static SlangResult _findLine(const UnownedStringSlice& find, UnownedStringSlice& ioRemaining, UnownedStringSlice& outLine)
{
    // Find the start line
    UnownedStringSlice line;
    while (StringUtil::extractLine(ioRemaining, line))
    {
        if (line.startsWith(find))
        {
            outLine = line;
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */SlangResult HexDumpUtil::findStartAndEndLines(const UnownedStringSlice& lines, UnownedStringSlice& outStart, UnownedStringSlice& outEnd)
{
    UnownedStringSlice remaining(lines);
    SLANG_RETURN_ON_FAIL(_findLine(s_start, remaining, outStart));
    SLANG_RETURN_ON_FAIL(_findLine(s_end, remaining, outEnd));
    return SLANG_OK;
}

/* static */SlangResult HexDumpUtil::parseWithMarkers(const UnownedStringSlice& lines, List<uint8_t>& outBytes)
{
    UnownedStringSlice startLine, endLine;
    SLANG_RETURN_ON_FAIL(findStartAndEndLines(lines, startLine, endLine));

    HashCode32 hash;
    size_t size;
    {
        // Get the size and the hash
        List<UnownedStringSlice> slices;
        StringUtil::split(startLine, ' ', slices);
        if (slices.getCount() != 3)
        {
            return SLANG_FAIL;
        }
        // Extract the size
        size = StringToInt(String(slices[1]));
        hash = HashCode32(StringToInt(String(slices[2])));
    }

    SLANG_RETURN_ON_FAIL(parse(UnownedStringSlice(startLine.end(), endLine.begin()), outBytes));

    // Calc the hash
    const HashCode32 readHash = getStableHashCode32((const char*)outBytes.begin(), outBytes.getCount());

    if (readHash != hash || size_t(outBytes.getCount()) != size)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

}
