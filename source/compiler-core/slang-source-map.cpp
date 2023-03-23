#include "slang-source-map.h"

#include "../../slang-com-helper.h"

#include "../core/slang-string-util.h"

#include "slang-json-native.h"

namespace Slang {

/*
Support for source maps. Source maps provide a standardized mechanism to associate a location in one output file
with another.

* [Source Map Proposal](https://docs.google.com/document/d/1U1RGAehQwRypUTovF1KRlpiOFze0b-_2gc6fAH0KY0k/edit?hl=en_US&pli=1&pli=1)
* [Chrome Source Map post](https://developer.chrome.com/blog/sourcemaps/)
* [Base64 VLQs in Source Maps](https://www.lucidchart.com/techblog/2019/08/22/decode-encoding-base64-vlqs-source-maps/)

Example...

{
"version" : 3,
"file": "out.js",
"sourceRoot": "",
"sources": ["foo.js", "bar.js"],
"sourcesContent": [null, null],
"names": ["src", "maps", "are", "fun"],
"mappings": "A,AAAB;;ABCDE;"
}
*/

namespace { // anonymous

struct JSONSourceMap
{
    /// File version (always the first entry in the object) and must be a positive integer.
    int32_t version = 3;
    /// An optional name of the generated code that this source map is associated with.
    String file;
    /// An optional source root, useful for relocating source files on a server or removing repeated values in 
    /// the “sources” entry.  This value is prepended to the individual entries in the “source” field.
    String sourceRoot;
    /// A list of original sources used by the “mappings” entry.
    List<UnownedStringSlice> sources;
    /// An optional list of source content, useful when the “source” can’t be hosted. The contents are listed in the same order as the sources in line 5. 
    /// “null” may be used if some original sources should be retrieved by name.
    /// Because could be a string or nullptr, we use JSONValue to hold value.
    List<JSONValue> sourcesContent;
    /// A list of symbol names used by the “mappings” entry.
    List<UnownedStringSlice> names;
    /// A string with the encoded mapping data.
    UnownedStringSlice mappings;

    static const StructRttiInfo g_rttiInfo;
};

} // anonymous

static const StructRttiInfo _makeJSONSourceMap_Rtti()
{
    JSONSourceMap obj;

    StructRttiBuilder builder(&obj, "SourceMap", nullptr);

    builder.addField("version", &obj.version);
    builder.addField("file", &obj.file);
    builder.addField("sourceRoot", &obj.sourceRoot, StructRttiInfo::Flag::Optional);
    builder.addField("sources", &obj.sources);
    builder.addField("sourcesContent", &obj.sourcesContent, StructRttiInfo::Flag::Optional);
    builder.addField("names", &obj.names, StructRttiInfo::Flag::Optional);
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
    SLANG_FORCE_INLINE int8_t operator[](char c) const { return (c & ~char(0x7f)) ? -1 : map[c]; }

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

    // Handle negating
    out = (v & 1) ? -(v >> 1) : (v >> 1);
    return SLANG_OK;
}

void _encode(Index v, StringBuilder& out)
{
    // Double to free up low bit to hold the sign
    v += v;

    // We want to make v always positive to encode
    // we use the last bit to indicate negativity
    v = (v < 0) ? (1 - v) : v; 
    
    // We'll use a simple buffer, so as to not have to constantly update he StringBuffer
    char dst[8];
    char* cur = dst;

    do 
    {
        // Encode it
        const auto nextV  = v >> 5;

        // Encode 5 bits
        char c = g_vlqEncodeTable[(v & 0x1f)];

        // See what bits are remaining
        v = (v >> 5);

        // Set the continuation bit's if there is more to encode
        c |= v ? 0x20 : 0;

        // Save the char
        *cur++ = c;
    }
    while (v);

    out.append(dst, cur);
}

void SourceMap::clear()
{
    String empty;

    m_file = empty;
    m_sourceRoot = empty;

    m_sources.clear();

    m_names.clear();

    m_sourcesContent.clear();

    m_lineStarts.setCount(1);
    m_lineStarts[0] = 0;

    m_lineEntries.clear();

    m_slicePool.clear();
}

void SourceMap::advanceToLine(Index nextLineIndex)
{
    const Count currentLineIndex = getGeneratedLineCount() - 1;

    SLANG_ASSERT(nextLineIndex >= currentLineIndex);
    
    if (nextLineIndex <= currentLineIndex)
    {
        return;
    }

    const auto lastEntryIndex = m_lineEntries.getCount();

    // For all the new entries they will need to point to the end 
    m_lineStarts.setCount(nextLineIndex + 1);

    Index* starts = m_lineStarts.getBuffer();
    for (Index i = currentLineIndex + 1; i < nextLineIndex + 1; ++i)
    {
        starts[i] = lastEntryIndex;
    }
}

Index SourceMap::getNameIndex(const UnownedStringSlice& slice)
{
    StringSlicePool::Handle handle;

    if (!m_slicePool.findOrAdd(slice, handle))
    {
        // We know it can't possibly be used, so must be new (!)

        m_names.add(handle);
        return m_names.getCount() - 1;
    }

    // Okay, could already be in the list
    const auto index = m_names.indexOf(handle);
    if (index >= 0)
    {
        return index;
    }

    m_names.add(handle);
    return m_names.getCount() - 1;
}

Index SourceMap::getSourceFileIndex(const UnownedStringSlice& slice)
{
    StringSlicePool::Handle handle;

    if (!m_slicePool.findOrAdd(slice, handle))
    {
        // We know it can't possibly be used, so must be new (!)

        m_sources.add(handle);
        return m_sources.getCount() - 1;
    }

    // Okay, could already be in the list
    const auto index = m_sources.indexOf(handle);
    if (index >= 0)
    {
        return index;
    }

    m_sources.add(handle);
    return m_sources.getCount() - 1;
}

SlangResult SourceMap::decode(JSONContainer* container, JSONValue root, DiagnosticSink* sink)
{
    clear();

    // Let's try and decode the JSON into native types to make this easier...
    RttiTypeFuncsMap typeMap = JSONNativeUtil::getTypeFuncsMap();

    // Convert to native
    JSONSourceMap native;
    {
        JSONToNativeConverter converter(container, &typeMap, sink);

        // Convert to the native type
        SLANG_RETURN_ON_FAIL(converter.convert(root, GetRttiInfo<JSONSourceMap>::get(), &native));
    }

    m_file = native.file;
    m_sourceRoot = native.sourceRoot;

    const Count sourcesCount = native.sources.getCount();
    
    // These should all be unique, but for simplicity, we build a table
    m_sources.setCount(sourcesCount);
    for (Index i = 0; i < sourcesCount; ++i)
    {
        m_sources[i] = m_slicePool.add(native.sources[i]);
    }

    Count sourcesContentCount = native.sourcesContent.getCount();
    sourcesContentCount = std::min(sourcesContentCount, sourcesCount);

    m_sourcesContent.setCount(sourcesContentCount);
    for (auto& cur : m_sourcesContent)
    {
        cur = StringSlicePool::kNullHandle;
    }

    for (Index i = 0; i < sourcesContentCount; ++i)
    {
        auto value = native.sourcesContent[i];

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
    StringUtil::split(native.mappings, ';', lines);
    
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
                /* If present, an zero-based index into the "sources" list. This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence of this field, in which case the whole value is represented.
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
                    /* If present, the zero - based index into the "names" list associated with this segment.
                    This field is a base 64 VLQ relative to the previous occurrence of this field, unless this is the first occurrence 
                    of this field, in which case the whole value is represented.
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
            entry.nameIndex = nameIndex;

            m_lineEntries.add(entry);
        }
    }

    // Mark the end
    m_lineStarts[linesCount] = m_lineEntries.getCount();

    return SLANG_OK;
}

SlangResult SourceMap::encode(JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue)
{
    // Convert to native
    JSONSourceMap native;

    native.file = m_file;
    native.sourceRoot = m_sourceRoot;

    // Copy over the sources
    {
        const auto count = m_sources.getCount();
        native.sources.setCount(count);
        for (Index i = 0; i < count; ++i)
        {
            native.sources[i] = m_slicePool.getSlice(m_sources[i]);
        }
    }

    // Copy out the sourcesContent, care is needed around handling null
    {
        const auto count = m_sourcesContent.getCount();
        native.sourcesContent.setCount(count);
        for (Index i = 0; i < count; ++i)
        {
            const auto srcValue = m_sourcesContent[i];
            
            const JSONValue dstValue = (srcValue == StringSlicePool::kNullHandle) ?
                native.sourcesContent[i] = JSONValue::makeNull() :
                container->createString(m_slicePool.getSlice(srcValue));

            native.sourcesContent[i] = dstValue;
        }
    }

    // Copy out the names
    {
        const auto count = m_names.getCount();
        native.names.setCount(count);
        for (Index i = 0; i < count; ++i)
        {
            native.names[i] = m_slicePool.getSlice(m_names[i]);
        }
    }

    StringBuilder mappings;

    // Do the encoding!
    {
        const Count linesCount = getGeneratedLineCount();

        Index sourceFileIndex = 0;

        Index sourceLine = 0;
        Index sourceColumn = 0;
        Index nameIndex = 0;

        for (Index i = 0; i < linesCount; ++i)
        {
            // Add the semicolon to start the line
            if (i > 0)
            {
                mappings.appendChar(';');
            }

            const auto entries = getEntriesForLine(i);
            const auto entriesCount = entries.getCount();

            if (entriesCount == 0)
            {
                continue;
            }

            // We reset the generated column index at the start of each new generated line
            Index generatedColumn = 0;
            
            for (Index j = 0; j < entriesCount; ++j)
            {
                auto entry = entries[j];

                if (j > 0)
                {
                    mappings.appendChar(',');
                }

                Index generatedDelta = entry.generatedColumn - generatedColumn;
                generatedColumn = entry.generatedColumn;

                _encode(generatedDelta, mappings);

                // See if there any other deltas we need to handle
                const Index sourceFileDelta = entry.sourceFileIndex - sourceFileIndex;
                const Index sourceLineDelta = entry.sourceLine - sourceLine;
                const Index sourceColumnDelta = entry.sourceColumn - sourceColumn;
                const Index nameIndexDelta = entry.nameIndex - nameIndex;

                if (sourceFileDelta || sourceLineDelta || sourceColumnDelta || nameIndex)
                {
                    // Okay we have to encode all these deltae
                    _encode(sourceFileDelta, mappings);
                    _encode(sourceLineDelta, mappings);
                    _encode(sourceColumnDelta, mappings);

                    // Update these values
                    sourceFileIndex = entry.sourceFileIndex;
                    sourceLine = entry.sourceLine;
                    sourceColumn = entry.sourceColumn;

                    if (nameIndexDelta)
                    {
                        _encode(nameIndexDelta, mappings);
                        nameIndex = entry.nameIndex;
                    }
                }
            }
        }
    }

    // Set the mappings
    native.mappings = mappings.getUnownedSlice();

    // Write it out
    {
        RttiTypeFuncsMap typeMap = JSONNativeUtil::getTypeFuncsMap();

        NativeToJSONConverter converter(container, &typeMap, sink);
        SLANG_RETURN_ON_FAIL(converter.convert(GetRttiInfo<JSONSourceMap>::get(), &native, outValue));
    }

    return SLANG_OK;
}

} // namespace Slang
