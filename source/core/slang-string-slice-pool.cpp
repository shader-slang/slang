#include "slang-string-slice-pool.h"

namespace Slang {

StringSlicePool::StringSlicePool() :
    m_arena(1024)
{
    clear();
}

void StringSlicePool::clear()
{
    m_slices.SetSize(1);
    m_slices[0] = UnownedStringSlice::fromLiteral("");

    m_map.Clear();
}

StringSlicePool::Handle StringSlicePool::add(const Slice& slice)
{
    const int* indexPtr = m_map.TryGetValue(slice);
    if (indexPtr)
    {
        return Handle(*indexPtr);
    }

    // Create a scoped copy
    UnownedStringSlice scopePath(m_arena.allocateString(slice.begin(), slice.size()), slice.size());

    const int index = int(m_slices.Count());

    m_slices.Add(scopePath);
    m_map.Add(scopePath, index);
    return Handle(index);
}

int StringSlicePool::findIndex(const Slice& slice) const
{
    const int* index = m_map.TryGetValue(slice);
    return index ? *index : -1;

}
} // namespace Slang
