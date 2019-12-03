#ifndef SLANG_CORE_STRING_SLICE_POOL_H
#define SLANG_CORE_STRING_SLICE_POOL_H

#include "slang-string.h"

#include "slang-list.h"
#include "slang-memory-arena.h"
#include "slang-dictionary.h"
#include "slang-array-view.h"

namespace Slang {

/* Holds a unique set of slices.
NOTE: that all slices are stored with terminating zeros.
*/

class StringSlicePool
{
public:
    typedef StringSlicePool ThisType;
    typedef uint32_t HandleIntegral;

    enum class Style
    {
        Default,            ///< Default style - has default handles (like kNullHandle and kEmptyHandle)
        Empty,              ///< Empty style - has no handles by default
    };

    /// Handle of 0 is null. If accessed will be returned as the empty string
    enum class Handle : HandleIntegral;
    typedef UnownedStringSlice Slice;

    static const Handle kNullHandle = Handle(0);
    static const Handle kEmptyHandle = Handle(1);

    static const Index kDefaultHandlesCount = 2;

        /// Returns the index of a slice, if contained, or -1 if not found
    Index findIndex(const Slice& slice) const;

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
    Index getSlicesCount() const { return m_slices.getCount(); }

        /// Returns true if the handle is a default one. Only meaningful on a Style::Default.
    bool isDefaultHandle(Handle handle) const { SLANG_ASSERT(m_style == Style::Default && Index(handle) >= 0); return Index(handle) < kDefaultHandlesCount; }

        /// Convert a handle to and index. (A handle is just an index!) 
    static Index asIndex(Handle handle) { return Index(handle); }
    
        /// Get the style of the pool
    Style getStyle() const { return m_style; }

        /// Get all the added slices
    ConstArrayView<UnownedStringSlice> getAdded() const; 

        /// Ctor
    StringSlicePool(Style style);

protected:
    // Disable copy ctor and assignment
    StringSlicePool(const ThisType& rhs) = delete;
    void operator=(const ThisType& rhs) = delete;

    Style m_style;
    List<UnownedStringSlice> m_slices;
    Dictionary<UnownedStringSlice, Handle> m_map;
    MemoryArena m_arena;
};

} // namespace Slang

#endif // SLANG_STRING_SLICE_POOL_H
