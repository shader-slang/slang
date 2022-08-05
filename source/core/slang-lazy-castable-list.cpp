// slang-lazy-castable-list.cpp
#include "slang-lazy-castable-list.h"

#include "slang-castable-list-impl.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LazyCastableList !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void LazyCastableList::removeAt(Index index)
{
    SLANG_ASSERT(index >= 0 && index < getCount());

    if (auto list = as<ICastableList>(m_castable))
    {
        list->removeAt(index);
    }
    else
    {
        SLANG_ASSERT(index == 0);
        m_castable.setNull();
    }
}

void LazyCastableList::clear()
{
    if (m_castable)
    {
        if (auto list = as<ICastableList>(m_castable))
        {
            list->clear();
        }
        else
        {
            m_castable.setNull();
        }
    }
}

void LazyCastableList::clearAndDeallocate()
{
    m_castable.setNull();
}

Count LazyCastableList::getCount() const
{
    if (m_castable)
    {
        if (auto list = as<ICastableList>(m_castable))
        {
            return list->getCount();
        }
        return 1;
    }
    return 0;
}

void LazyCastableList::add(ICastable* castable)
{
    SLANG_ASSERT(as<ICastableList>(castable) == nullptr);
    SLANG_ASSERT(castable);
    SLANG_ASSERT(castable != m_castable);

    if (m_castable)
    {
        if (auto list = as<ICastableList>(m_castable))
        {
            // Shouldn't be in the list
            SLANG_ASSERT(list->indexOf(castable) < 0);
            list->add(castable);
        }
        else
        {
            list = new CastableList;
            list->add(m_castable);
            m_castable = list;
            list->add(castable);
        }
    }
    else
    {
        m_castable = castable;
    }
}

ICastableList* LazyCastableList::requireList()
{
    if (m_castable)
    {
        if (auto list = as<ICastableList>(m_castable))
        {
            return list;
        }
        else
        {
            // Promote to a list with the element in it
            list = new CastableList;
            list->add(m_castable);
            m_castable = list;
            return list;
        }
    }
    else
    {
        // Create an empty list
        ICastableList* list = new CastableList;
        m_castable = list;
        return list;
    }
}

ICastableList* LazyCastableList::getList()
{
    return (m_castable == nullptr) ? nullptr : requireList();
}

void* LazyCastableList::find(const Guid& guid)
{
    if (!m_castable)
    {
        return nullptr;
    }
    if (auto list = as<ICastableList>(m_castable))
    {
        return list->find(guid);
    }
    else
    {
        return m_castable->castAs(guid);
    }
}

ConstArrayView<ICastable*> LazyCastableList::getView() const
{
    if (!m_castable)
    {
        // Empty
        return ConstArrayView<ICastable*>();
    }

    if (auto list = as<ICastableList>(m_castable))
    {
        const auto count = list->getCount();
        const auto buffer = list->getBuffer();

        return ConstArrayView<ICastable*>(buffer, count);
    }
    else
    {
        return ConstArrayView<ICastable*>((ICastable*const*)&m_castable, 1);
    }
}

Index LazyCastableList::indexOf(ICastable* castable) const
{
    return getView().indexOf(castable);
}

Index LazyCastableList::indexOfUnknown(ISlangUnknown* unk) const
{
    {
        ComPtr<ICastable> castable;
        if (SLANG_SUCCEEDED(unk->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            return indexOf(castable);
        }
    }

    // It's not derived from ICastable, so can only be in list via an adapter
    const auto view = getView();

    const Count count = view.getCount();
    for (Index i = 0; i < count; ++i)
    {
        auto adapter = as<IUnknownCastableAdapter>(view[i]);
        if (adapter && adapter->getContained() == unk)
        {
            return i;
        }
    }

    return -1;
}

} // namespace Slang
