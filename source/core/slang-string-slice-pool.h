#ifndef SLANG_STRING_SLICE_POOL_H
#define SLANG_STRING_SLICE_POOL_H

#include "slang-string.h"

#include "list.h"
#include "slang-memory-arena.h"
#include "dictionary.h"

namespace Slang {

class StringSlicePool
{
public:
    /// Handle of 0 is null. If accessed will be returned as the empty string
    enum class Handle : uint32_t;
    typedef UnownedStringSlice Slice;

        /// Returns the index of a slice, if contained, or -1 if not found
    int findIndex(const Slice& slice) const;

        /// True if has the slice
    bool has(const Slice& slice) { return findIndex(slice) >= 0; }
        /// Add a slice 
    Handle add(const Slice& slice);

        /// Empty contents
    void clear();

        /// Get the slice from the handle
    const UnownedStringSlice& getSlice(Handle handle) const { return m_slices[UInt(handle)]; }

        /// Get all the slices
    const List<UnownedStringSlice>& getSlices() const { return m_slices; }

        /// Convert a handle to and index. (A handle is just an index!) 
    static int asIndex(Handle handle) { return int(handle); }

        /// Ctor
    StringSlicePool();

protected:
    List<UnownedStringSlice> m_slices;
    Dictionary<UnownedStringSlice, int> m_map;
    MemoryArena m_arena;
};

} // namespace Slang

#endif // SLANG_STRING_SLICE_POOL_H
