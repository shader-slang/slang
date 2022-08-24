// slang-slice-allocator.cpp
#include "slang-slice-allocator.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SliceConverter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */ List<String> SliceConverter::toList(const Slice<TerminatedCharSlice>& in)
{
    List<String> list;
    const auto count = in.count;

    list.setCount(count);
    for (Index i = 0; i < count; ++i)
    {
        list[i] = asStringSlice(in[i]);
    }
    return list;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SliceAllocator !!!!!!!!!!!!!!!!!!!!!!!!!!! */

TerminatedCharSlice SliceAllocator::allocate(const char* in)
{
    const size_t length = ::strlen(in);
    auto dst = m_arena.allocateString(in, length);
    return TerminatedCharSlice(dst, length);
}

TerminatedCharSlice SliceAllocator::allocate(const UnownedStringSlice& slice)
{
    const auto length = slice.getLength();
    auto dst = m_arena.allocateString(slice.begin(), length);
    return TerminatedCharSlice(dst, length);
}

TerminatedCharSlice SliceAllocator::allocate(const Slice<char>& slice)
{
    const auto count = slice.count;
    auto dst = m_arena.allocateString(slice.begin(), count);
    return TerminatedCharSlice(dst, count);
}

Slice<TerminatedCharSlice> SliceAllocator::allocate(const List<String>& in)
{
    const auto count = in.getCount();
    if (count == 0)
    {
        return Slice<TerminatedCharSlice>(nullptr, 0);
    }

    auto dst = m_arena.allocateArray<TerminatedCharSlice>(count);
    for (Index i = 0; i < count; ++i)
    {
        dst[i] = allocate(in[i]);
    }

    return Slice<TerminatedCharSlice>(dst, count);
}

} // namespace Slang
