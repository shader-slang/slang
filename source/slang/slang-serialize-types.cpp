// slang-serialize-types.cpp
#include "slang-serialize-types.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "../core/slang-math.h"

namespace Slang {

// Needed for linkage with some compilers
/* static */ const SerialStringData::StringIndex SerialStringData::kNullStringIndex;
/* static */ const SerialStringData::StringIndex SerialStringData::kEmptyStringIndex;

namespace { // anonymous

struct CharReader
{
    char operator()(int pos) const { SLANG_UNUSED(pos); return *m_pos++; }
    CharReader(const char* pos) :m_pos(pos) {}
    mutable const char* m_pos;
};

} // anonymous

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialStringTable !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SerialStringTable::SerialStringTable():
    m_stringTable(nullptr)
{
}

void SerialStringTable::init(const List<char>* stringTable)
{
    m_stringTable = stringTable;

    // Decode the table
    m_entries.setCount(StringSlicePool::kDefaultHandlesCount);
    SLANG_COMPILE_TIME_ASSERT(StringSlicePool::kDefaultHandlesCount == 2);

    {
        Entry& entry = m_entries[0];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
    }
    {
        Entry& entry = m_entries[1];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
    }

    {
        const char* start = stringTable->begin();
        const char* cur = start;
        const char* end = stringTable->end();

        while (cur < end)
        {
            CharReader reader(cur);
            const int len = GetUnicodePointFromUTF8(reader);

            Entry entry;
            entry.m_startIndex = uint32_t(reader.m_pos - start);
            entry.m_numChars = len;

            m_entries.add(entry);

            cur = reader.m_pos + len;
        }
    }

    m_entries.compress();
}

UnownedStringSlice SerialStringTable::getStringSlice(Handle handle) const
{
    const Entry& entry = m_entries[int(handle)];
    const char* start = m_stringTable->begin();

    return UnownedStringSlice(start + entry.m_startIndex, int(entry.m_numChars));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialStringTableUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void SerialStringTableUtil::encodeStringTable(const StringSlicePool& pool, List<char>& stringTable)
{
    // Skip the default handles -> nothing is encoded via them
    return encodeStringTable(pool.getAdded(), stringTable);
}
    
/* static */void SerialStringTableUtil::encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable)
{
    stringTable.clear();
    for (const auto& slice : slices)
    {
        const int len = int(slice.getLength());
        
        // We need to write into the the string array
        char prefixBytes[6];
        const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
        const Index baseIndex = stringTable.getCount();

        stringTable.setCount(baseIndex + numPrefixBytes + len);

        char* dst = stringTable.begin() + baseIndex;

        memcpy(dst, prefixBytes, numPrefixBytes);
        memcpy(dst + numPrefixBytes, slice.begin(), len);   
    }
}

/* static */void SerialStringTableUtil::appendDecodedStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    const char* start = stringTable.begin();
    const char* cur = start;
    const char* end = stringTable.end();

    while (cur < end)
    {
        CharReader reader(cur);
        const int len = GetUnicodePointFromUTF8(reader);
        slicesOut.add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

/* static */void SerialStringTableUtil::decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.setCount(2);
    slicesOut[0] = UnownedStringSlice(nullptr, size_t(0));
    slicesOut[1] = UnownedStringSlice("", size_t(0));

    appendDecodedStringTable(stringTable, slicesOut);
}

/* static */void SerialStringTableUtil::calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMapOut)
{
    SLANG_ASSERT(slices.getCount() >= StringSlicePool::kDefaultHandlesCount);
    SLANG_ASSERT(slices[int(StringSlicePool::kNullHandle)] == "" && slices[int(StringSlicePool::kNullHandle)].begin() == nullptr);
    SLANG_ASSERT(slices[int(StringSlicePool::kEmptyHandle)] == "");

    indexMapOut.setCount(slices.getCount());
    // Set up all of the defaults
    for (int i = 0; i < StringSlicePool::kDefaultHandlesCount; ++i)
    {
        indexMapOut[i] = StringSlicePool::Handle(i);
    }

    const Index numSlices = slices.getCount();
    for (Index i = StringSlicePool::kDefaultHandlesCount; i < numSlices ; ++i)
    {
        indexMapOut[i] = pool.add(slices[i]);
    }
}

} // namespace Slang
