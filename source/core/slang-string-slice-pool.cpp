#include "slang-string-slice-pool.h"

namespace Slang {

/* static */ const StringSlicePool::Handle StringSlicePool::kNullHandle;
/* static */ const StringSlicePool::Handle StringSlicePool::kEmptyHandle;

/* static */const int StringSlicePool::kNumDefaultHandles;


StringSlicePool::StringSlicePool() :
    m_arena(1024)
{
    clear();
}

void StringSlicePool::clear()
{
    m_slices.setCount(2);

    m_slices[0] = UnownedStringSlice((const char*)nullptr, (const char*)nullptr);
    m_slices[1] = UnownedStringSlice::fromLiteral("");
    
    // Add the empty entry
    m_map.Add(m_slices[1], kEmptyHandle);

    m_map.Clear();
}

StringSlicePool::Handle StringSlicePool::add(const Slice& slice)
{
    const Handle* handlePtr = m_map.TryGetValue(slice);
    if (handlePtr)
    {
        return *handlePtr;
    }

    // Create a scoped copy
    UnownedStringSlice scopePath(m_arena.allocateString(slice.begin(), slice.size()), slice.size());

    const auto index = m_slices.getCount();

    m_slices.add(scopePath);
    m_map.Add(scopePath, Handle(index));
    return Handle(index);
}

StringSlicePool::Handle StringSlicePool::add(StringRepresentation* stringRep)
{
    if (stringRep == nullptr)
    {
        return kNullHandle;
    }
    return add(StringRepresentation::asSlice(stringRep));
}
 

StringSlicePool::Handle StringSlicePool::add(const char* chars)
{
    if (!chars)
    {
        return kNullHandle;
    }
    if (chars[0] == 0)
    {
        return kEmptyHandle;
    }
    return add(UnownedStringSlice(chars));
}

int StringSlicePool::findIndex(const Slice& slice) const
{
    const Handle* handlePtr = m_map.TryGetValue(slice);
    return handlePtr ? int(*handlePtr) : -1;

}
} // namespace Slang
