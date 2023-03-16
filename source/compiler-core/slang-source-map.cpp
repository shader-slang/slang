#include "slang-source-map.h"

#include "../../slang-com-helper.h"

#include "../core/slang-string-util.h"

#include "slang-json-native.h"

namespace Slang {

static const StructRttiInfo _makeJSONSourceMap_Rtti()
{
    JSONSourceMap obj;

    StructRttiBuilder builder(&obj, "SourceMap", nullptr);

    builder.addField("version", &obj.version);
    builder.addField("file", &obj.file);
    builder.addField("sourceRoot", &obj.sourceRoot);
    builder.addField("sources", &obj.sources);
    builder.addField("sourcesContent", &obj.sourcesContent);
    builder.addField("names", &obj.names);
    builder.addField("mappings", &obj.mappings);

    return builder.make();
}
/* static */const StructRttiInfo JSONSourceMap::g_rttiInfo = _makeJSONSourceMap_Rtti();

// Encode a 6 bit value to VLQ encoding
static const char g_vlqEncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct VlqDecodeTable
{
    VlqDecodeTable()
    {
        ::memset(map, -1, sizeof(map));
        for (Index i = 0; i < SLANG_COUNT_OF(g_vlqEncodeTable); ++i)
        {
            map[g_vlqEncodeTable[i]] = int8_t(i);
        }
    }
        /// Returns a *negative* value if invalid
    int8_t operator[](char c) const { return (c & ~char(0x7f)) ? -1 : map[c]; }

    int8_t map[128];
};

static const VlqDecodeTable g_vlqDecodeTable;

/* 
https://docs.google.com/document/d/1U1RGAehQwRypUTovF1KRlpiOFze0b-_2gc6fAH0KY0k/edit?hl=en_US&pli=1&pli=1#
The VLQ is a Base64 value, where the most significant bit (the 6th bit) is used as the continuation 
bit, and the “digits” are encoded into the string least significant first, and where the least significant 
bit of the first digit is used as the sign bit. */

static SlangResult _decode(UnownedStringSlice& ioEncoded, Index& out)
{
    Index v = 0;

    Index shift = 0;
    const char* cur = ioEncoded.begin();
    const char* end = ioEncoded.end();

    // Must have some chars
    if (cur >= end)
    {
        return SLANG_FAIL;
    }

    for (; cur < end; ++cur)
    {
        const Index value = g_vlqDecodeTable[*cur];
        if (value < 0)
        {
            return SLANG_FAIL;
        }
        
        v += (value & 0x1f) << shift;

        // If the continuation bit is not set we are done
        if (( value & 0x20) == 0)
        {
            ++cur;
            break;
        }

        shift += 5;
    }

    // Save out the remaining part
    ioEncoded = UnownedStringSlice(cur, end);

    // Double to make setting lower bit simpler
    v += v;

    // If it's negative we make positive and set the bottom bit
    // otherwise we just return with the LSB not set.
    out = (v < 0) ? (1 - v) : v;
    return SLANG_OK;
}

SlangResult SourceMap::decode(JSONContainer* container, const JSONSourceMap& src)
{
    m_slicePool.clear();

    m_file = src.file;
    m_sourceRoot = src.sourceRoot;

    m_lineStarts.clear();
    m_lineEntries.clear();

    const Count sourcesCount = src.sources.getCount();
    
    // These should all be unique, but for simplicity, we build a table
    m_sources.setCount(sourcesCount);
    for (Index i = 0; i < sourcesCount; ++i)
    {
        m_sources[i] = m_slicePool.add(src.sources[i]);
    }

    Count sourcesContentCount = src.sourcesContent.getCount();
    sourcesContentCount = std::min(sourcesContentCount, sourcesCount);

    m_sourcesContent.setCount(sourcesContentCount);
    for (auto& cur : m_sourcesContent)
    {
        cur = StringSlicePool::kNullHandle;
    }

    for (Index i = 0; i < sourcesContentCount; ++i)
    {
        auto value = src.sourcesContent[i];

        if (value.type != JSONValue::Type::Null)
        {
            if (value.getKind() == JSONValue::Kind::String)
            {
                auto stringValue = container->getString(value);
                m_sourcesContent[i] = m_slicePool.add(stringValue);
            }
        }
    }

    
    List<UnownedStringSlice> lines;
    StringUtil::split(src.mappings, ';', lines);
    
    List<UnownedStringSlice> segments;

    // Index into sources 
    Index sourceFileIndex = 0;

    Index sourceLine = 0;
    Index sourceColumn = 0;
    Index nameIndex = 0;

    const Count linesCount = lines.getCount();

    m_lineStarts.setCount(linesCount + 1);

    for (Index generatedLine = 0; generatedLine < linesCount; ++generatedLine)
    {
        const auto line = lines[generatedLine];

        m_lineStarts[generatedLine] = m_lineEntries.getCount();

        // If it's empty move to next line
        if (line.getLength() == 0)
        {
            continue;
        }

        // Split the line into segments
        segments.clear();
        StringUtil::split(line, ',', segments);

        Index generatedColumn = 0;

        for (auto segment : segments)
        {
            Index colDelta;
            SLANG_RETURN_ON_FAIL(_decode(segment, colDelta));

            generatedColumn += colDelta;
            SLANG_ASSERT(generatedColumn >= 0);

            // It can be 4 or 5 parts
            if (segment.getLength())
            {
                /* If present, an zero-based index into the “sources” list. This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence of this field, in which case the whole value is represented.
                    If present, the zero-based starting line in the original source represented. This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence of this field, in which case the whole value is represented. Always present if there is a source field.
                    If present, the zero-based starting column of the line in the source represented. This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence of this field, in which case the whole value is represented. Always present if there is a source field.
                     */

                Index sourceFileDelta;
                Index sourceLineDelta;
                Index sourceColumnDelta;

                SLANG_RETURN_ON_FAIL(_decode(segment, sourceFileDelta));
                SLANG_RETURN_ON_FAIL(_decode(segment, sourceLineDelta));
                SLANG_RETURN_ON_FAIL(_decode(segment, sourceColumnDelta));

                sourceFileIndex += sourceFileDelta;
                sourceLine += sourceLineDelta;
                sourceColumn += sourceColumnDelta;

                SLANG_ASSERT(sourceFileIndex >= 0);
                SLANG_ASSERT(sourceLine >= 0);
                SLANG_ASSERT(sourceColumn >= 0);

                // 5 parts
                if (segment.getLength() > 0)
                {
                    /* If present, the zero - based index into the “names” list associated with this segment.This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence of this field, in which case the whole value is represented.
                    */

                    Index nameDelta;
                    SLANG_RETURN_ON_FAIL(_decode(segment, nameDelta));

                    nameIndex += nameDelta;
                    SLANG_ASSERT(nameIndex >= 0);
                }
            }

            Entry entry;
            entry.generatedColumn = generatedColumn;
            entry.sourceColumn = sourceColumn;
            entry.sourceLine = sourceLine;
            entry.sourceFileIndex = sourceFileIndex;

            m_lineEntries.add(entry);
        }
    }

    // Mark the end
    m_lineStarts[linesCount] = m_lineEntries.getCount();

    return SLANG_OK;
}


} // namespace Slang
