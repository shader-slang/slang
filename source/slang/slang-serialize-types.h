// slang-serialize-types.h
#ifndef SLANG_SERIALIZE_TYPES_H
#define SLANG_SERIALIZE_TYPES_H

#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-array-view.h"

#include "slang-name.h"
#include "slang-source-loc.h"

namespace Slang {

struct SerialStringData
{
    enum class StringIndex : uint32_t;
 
    ///enum class StringOffset : uint32_t;                     ///< Offset into the m_stringsBuffer

    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(StringSlicePool::kNullHandle);
    static const StringIndex kEmptyStringIndex = StringIndex(StringSlicePool::kEmptyHandle);
};

class SerialStringTable
{
    public:
    typedef StringSlicePool::Handle Handle;

    struct Entry
    {
        uint32_t m_startIndex;
        uint32_t m_numChars;
    };

        /// Get as a string slice
    UnownedStringSlice getStringSlice(Handle handle) const;

        /// Initialize a cache to use a string table
    void init(const List<char>* stringTable);

        /// Ctor
    SerialStringTable(); 
    
    protected:
    const List<char>* m_stringTable;
    List<Entry> m_entries;
};

struct SerialStringTableUtil
{
        /// Convert a pool into a string table
    static void encodeStringTable(const StringSlicePool& pool, List<char>& stringTable);
    static void encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable);

        /// Appends the decoded strings into slicesOut
    static void appendDecodedStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut);
        /// Decodes a string table (and does so such that the indices are compatible with StringSlicePool)
    static void decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut);

        /// Produces an index map, from slices to indices in pool
    static void calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMap);
};

} // namespace Slang

#endif
