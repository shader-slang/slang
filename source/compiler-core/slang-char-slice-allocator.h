// slang-char-slice-allocator.h
#ifndef SLANG_CHAR_SLICE_ALLOCATOR_H
#define SLANG_CHAR_SLICE_ALLOCATOR_H

// Has definition of CharSlice 
#include "slang-artifact.h"

#include "../core/slang-memory-arena.h"

namespace Slang
{

/* 
The reason to wrap in a struct rather than have as free functions is doing so will lead to compile time 
errors with incorrect usage around temporaries.
*/
struct CharSliceCaster
{
        /// The slice will only be in scope whilst the string is
    static TerminatedCharSlice asTerminatedCharSlice(const String& in) { auto unowned = in.getUnownedSlice(); return TerminatedCharSlice(unowned.begin(), unowned.getLength()); }

    static CharSlice asCharSlice(const String& in) { auto unowned = in.getUnownedSlice(); return CharSlice(unowned.begin(), unowned.getLength()); }

        /// Convert into a list of strings
    static List<String> asList(const Slice<TerminatedCharSlice>& in);

    template <typename T>
    static List<ComPtr<T>> asComPtrList(const Slice<T*>& in) 
    { 
        ISlangUnknown* check = (T*)nullptr;
        SLANG_UNUSED(check);
        List<ComPtr<T>> list; 
        list.setCount(in.count);
        for (Index i = 0; i < in.count; ++i) list[i] = ComPtr<T>(in[i]);
        return list; 
    }
    template <typename T>
    static Slice<T*> asSlice(const List<ComPtr<T>>& list) { return makeSlice((T*const*)list.getBuffer(), list.getCount()); }

private:
        // We don't want temporaries to be 'asSliced' so disable
    static TerminatedCharSlice asTerminatedCharSlice(const String&& in) = delete;
    static CharSlice asCharSlice(const String&& in) = delete;
};

SLANG_FORCE_INLINE UnownedStringSlice asStringSlice(const CharSlice& slice)
{
    return UnownedStringSlice(slice.begin(), slice.end());
}

SLANG_FORCE_INLINE CharSlice asCharSlice(const UnownedStringSlice& slice)
{
    return CharSlice(slice.begin(), slice.getLength());
}

SLANG_FORCE_INLINE String asString(const CharSlice& slice)
{
    return String(slice.begin(), slice.end()); 
}

struct CharSliceAllocator
{
    TerminatedCharSlice allocate(const Slice<char>& slice);
    TerminatedCharSlice allocate(const UnownedStringSlice& slice);
    TerminatedCharSlice allocate(const String& in) { return allocate(in.getUnownedSlice()); }
    TerminatedCharSlice allocate(const char* in);
    TerminatedCharSlice allocate(const char* start, const char* end) { return allocate(UnownedStringSlice(start, end)); }

    Slice<TerminatedCharSlice> allocate(const List<String>& in);

        /// Get the backing arena
    MemoryArena& getArena() { return m_arena; }

    void deallocateAll() { m_arena.deallocateAll(); }

    CharSliceAllocator():
        m_arena(1024)
    {
    }
protected:
    
    MemoryArena m_arena;
};

} // namespace Slang

#endif
