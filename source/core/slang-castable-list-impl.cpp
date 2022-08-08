// slang-castable-list-impl.cpp
#include "slang-castable-list-impl.h"

namespace Slang {

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! UnknownCastableAdapter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* UnknownCastableAdapter::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    if (auto obj = getObject(guid))
    {
        return obj;
    }

    if (m_found && guid == m_foundGuid)
    {
        return m_found;
    }

    ComPtr<ISlangUnknown> cast;
    if (SLANG_SUCCEEDED(m_contained->queryInterface(guid, (void**)cast.writeRef())) && cast)
    {
        // Save the interface in the cache
        m_found = cast;
        m_foundGuid = guid;

        return cast;
    }
    return nullptr;
}

void* UnknownCastableAdapter::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid())
    {
        return static_cast<ICastable*>(this);
    }
    return nullptr;
}

void* UnknownCastableAdapter::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CastableList !!!!!!!!!!!!!!!!!!!!!!!!!!! */

CastableList::~CastableList()
{
    for (auto castable : m_list)
    {
        castable->release();
    }
}

void* CastableList::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void* CastableList::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == ICastableList::getTypeGuid())
    {
        return static_cast<ICastableList*>(this);
    }
    return nullptr;
}

void* CastableList::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* CastableList::find(const Guid& guid)
{
    for (ICastable* castable : m_list)
    {
        if (auto ptr = castable->castAs(guid))
        {
            return ptr;
        }
    }
    return nullptr;
}

ICastable* SLANG_MCALL CastableList::findWithPredicate(FindFunc func, void* data)
{
    for (ICastable* castable : m_list)
    {
        if (func(castable, data))
        {
            return castable;
        }
    }
    return nullptr;
}

Index CastableList::indexOf(ICastable* castable)
{
    const Count count = m_list.getCount();
    for (Index i = 0; i < count; ++i)
    {
        ICastable* cur = m_list[i];
        if (cur == castable)
        {
            return i;
        }
    }
    return -1;
}

void CastableList::add(ICastable* castable) 
{ 
    SLANG_ASSERT(castable);
    castable->addRef();
    m_list.add(castable);
}

void CastableList::removeAt(Index i) 
{ 
    auto castable = m_list[i];
    m_list.removeAt(i);
    castable->release();
}

void CastableList::clear() 
{ 
    for (auto castable : m_list)
    {
        castable->release();
    }
    m_list.clear(); 
}

void CastableList::addUnknown(ISlangUnknown* unk)
{
    // If it has ICastable interface we can just add as that
    {
        ComPtr<ICastable> castable;
        if (SLANG_SUCCEEDED(unk->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            return add(castable);
        }
    }

    // Wrap it in an adapter
    IUnknownCastableAdapter* adapter = new UnknownCastableAdapter(unk);
    add(adapter);
}

Index CastableList::indexOfUnknown(ISlangUnknown* unk)
{
    SLANG_ASSERT(unk);
    // If it has a castable interface we can just look for that
    {
        ComPtr<ICastable> castable;
        if (SLANG_SUCCEEDED(unk->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            return indexOf(castable);
        }
    }

    // It's not derived from ICastable, so can only be in list via an adapter
    const Count count = m_list.getCount();
    for (Index i = 0; i < count; ++i)
    {
        auto adapter = as<IUnknownCastableAdapter>(m_list[i]);
        if (adapter && adapter->getContained() == unk)
        {
            return i;
        }
    }
    return -1;
}

} // namespace Slang
