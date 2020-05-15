#include "slang-string-slice-pool.h"

namespace Slang {

/* static */ const StringSlicePool::Handle StringSlicePool::kNullHandle;
/* static */ const StringSlicePool::Handle StringSlicePool::kEmptyHandle;

/* static */const Index StringSlicePool::kDefaultHandlesCount;

StringSlicePool::StringSlicePool(Style style) :
    m_style(style),
    m_arena(1024)
{
    clear();
}

void StringSlicePool::clear()
{
    m_map.Clear();

    switch (m_style)
    {
        case Style::Default:
        {
            // Add the defaults
            m_slices.setCount(2);

            m_slices[0] = UnownedStringSlice((const char*)nullptr, (const char*)nullptr);
            m_slices[1] = UnownedStringSlice::fromLiteral("");
            
            // Add the empty entry
            m_map.Add(m_slices[1], kEmptyHandle);
            break;
        }
        case Style::Empty:
        {
            // There are no defaults
            m_slices.clear();
            break;
        }
    }
}

StringSlicePool::Handle StringSlicePool::add(const Slice& slice)
{
    const Handle* handlePtr = m_map.TryGetValue(slice);
    if (handlePtr)
    {
        return *handlePtr;
    }

    // Create a scoped copy
    UnownedStringSlice scopePath(m_arena.allocateString(slice.begin(), slice.getLength()), slice.getLength());

    const auto index = m_slices.getCount();

    m_slices.add(scopePath);
    m_map.Add(scopePath, Handle(index));
    return Handle(index);
}

bool StringSlicePool::findOrAdd(const Slice& slice, Handle& outHandle)
{
    const Handle* handlePtr = m_map.TryGetValue(slice);
    if (handlePtr)
    {
        outHandle = *handlePtr;
        return true;
    }

    // Need to add.

    // Make a copy stored in the arena
    UnownedStringSlice scopeSlice(m_arena.allocateString(slice.begin(), slice.getLength()), slice.getLength());

    // Add using the arenas copy
    Handle newHandle = Handle(m_slices.getCount());
    m_map.Add(scopeSlice, newHandle);

    // Add to slices list
    m_slices.add(scopeSlice);
    outHandle = newHandle;
    return false;
}

StringSlicePool::Handle StringSlicePool::add(StringRepresentation* stringRep)
{
    if (stringRep == nullptr && m_style == Style::Default)
    {
        return kNullHandle;
    }
    return add(StringRepresentation::asSlice(stringRep));
}
 
StringSlicePool::Handle StringSlicePool::add(const char* chars)
{
    switch (m_style)
    {
        case Style::Default:
        {
            if (!chars)
            {
                return kNullHandle;
            }
            if (chars[0] == 0)
            {
                return kEmptyHandle;
            }
            break;
        }
        case Style::Empty:
        {
            if (chars == nullptr)
            {
                SLANG_ASSERT(!"Empty style doesn't support nullptr");
                // Return an invalid handle
                return Handle(~HandleIntegral(0));
            }
        }
    }
    
    return add(UnownedStringSlice(chars));
}

Index StringSlicePool::findIndex(const Slice& slice) const
{
    const Handle* handlePtr = m_map.TryGetValue(slice);
    return handlePtr ? Index(*handlePtr) : -1;
}

ConstArrayView<UnownedStringSlice> StringSlicePool::getAdded() const
{
    const Index firstIndex = getFirstAddedIndex();
    return makeConstArrayView(m_slices.getBuffer() + firstIndex, m_slices.getCount() - firstIndex);
}

} // namespace Slang
