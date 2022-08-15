// slang-lazy-castable-list.cpp
#include "slang-lazy-castable-list.h"

#include "slang-castable-list-impl.h"

namespace Slang {

void LazyCastableList::removeAt(Index index)
{
    SLANG_ASSERT(index >= 0 && index < getCount());

    switch (m_state)
    {
        case State::None:   break;
        case State::One:    
        {
            m_state = State::None;
            m_castable.setNull();
            break;
        }
        case State::List:   
        {
            static_cast<ICastableList*>(m_castable.get())->removeAt(index);
            break;
        }
    }
}

void LazyCastableList::clear()
{
    if (m_state == State::List)
    {
        auto list = static_cast<ICastableList*>(m_castable.get());
        list->clear();
    }
    else
    {
        m_state = State::None;
        m_castable.setNull();
    }
}

void LazyCastableList::clearAndDeallocate()
{
    m_state = State::None;
    m_castable.setNull();
}

Count LazyCastableList::getCount() const
{
    switch (m_state)
    {
        case State::None:   return 0;
        case State::One:    return 1;
        default:
        case State::List:   return static_cast<ICastableList*>(m_castable.get())->getCount();
    }
}

void LazyCastableList::add(ICastable* castable)
{
    SLANG_ASSERT(castable);
    if (m_state == State::None)
    {
        m_castable = castable;
        m_state = State::One;
    }
    else
    {
        requireList()->add(castable);
    }
}

ICastableList* LazyCastableList::requireList()
{
    switch (m_state)
    {
        case State::None:
        {
            m_castable = new CastableList;
            m_state = State::List;
            break;
        }
        case State::One:
        {
            // Turn into a list
            auto list = new CastableList;
            list->add(m_castable);
            m_castable = list;
            m_state = State::List;
            break;
        }
        default: break;
    }
    SLANG_ASSERT(m_state == State::List);
    return static_cast<ICastableList*>(m_castable.get());
}

ICastableList* LazyCastableList::getList()
{
    return (m_state == State::None) ? nullptr : requireList();
}

void* LazyCastableList::find(const Guid& guid)
{
    for (auto castable : getView())
    {
        if (auto ptr = castable->castAs(guid))
        {
            return ptr;
        }
    }
    return nullptr;
}

ICastable* LazyCastableList::findWithPredicate(ICastableList::FindFunc func, void* data)
{
    for (auto castable : getView())
    {
        if (func(castable, data))
        {
            return castable;
        }
    }
    return nullptr;
}

ConstArrayView<ICastable*> LazyCastableList::getView() const
{
    switch (m_state)
    {
        case State::None: return ConstArrayView<ICastable*>();
        case State::One:  return ConstArrayView<ICastable*>((ICastable*const*)&m_castable, 1);
        default:
        case State::List:
        {
            auto list = static_cast<ICastableList*>(m_castable.get());
            return ConstArrayView<ICastable*>(list->getBuffer(), list->getCount());
        }
    }
}

Index LazyCastableList::indexOf(ICastable* castable) const
{
    return getView().indexOf(castable);
}

Index LazyCastableList::indexOfUnknown(ISlangUnknown* unk) const
{
    // Try as a ICastable first
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
