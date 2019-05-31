#ifndef SLANG_CORE_STRING_SLICE_POOL_H
#define SLANG_CORE_STRING_SLICE_POOL_H

#include "slang-string.h"

#include "slang-list.h"
#include "slang-memory-arena.h"
#include "slang-dictionary.h"

namespace Slang {

class StringSlicePool
{
public:

    /// Handle of 0 is null. If accessed will be returned as the empty string
    enum class Handle : uint32_t;
    typedef UnownedStringSlice Slice;

    static const Handle kNullHandle = Handle(0);
    static const Handle kEmptyHandle = Handle(1);

    static const int kNumDefaultHandles = 2;

        /// Returns the index of a slice, if contained, or -1 if not found
    int findIndex(const Slice& slice) const;

        /// True if has the slice
    bool has(const Slice& slice) { return findIndex(slice) >= 0; }
        /// Add a slice 
    Handle add(const Slice& slice);
        /// Add from a string
    Handle add(const char* chars);
        /// Add a StringRepresentation
    Handle add(StringRepresentation* string);
        /// Add a string
    Handle add(const String& string) { return add(string.getUnownedSlice()); }

        /// Empty contents
    void clear();

        /// Get the slice from the handle
    const UnownedStringSlice& getSlice(Handle handle) const { return m_slices[UInt(handle)]; }

        /// Get all the slices
    const List<UnownedStringSlice>& getSlices() const { return m_slices; }

        /// Get the number of slices
    int getNumSlices() const { return int(m_slices.getCount()); }

        /// Convert a handle to and index. (A handle is just an index!) 
    static int asIndex(Handle handle) { return int(handle); }
        /// Returns true if the handle is to a slice that contains characters (ie not null or empty)
    static bool hasContents(Handle handle) { return int(handle) >= kNumDefaultHandles; }

        /// Ctor
    StringSlicePool();

protected:
    List<UnownedStringSlice> m_slices;
    Dictionary<UnownedStringSlice, Handle> m_map;
    MemoryArena m_arena;
};

} // namespace Slang

#endif // SLANG_STRING_SLICE_POOL_H
