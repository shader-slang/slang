// slang-castable-list-impl.cpp
#include "slang-castable-list-impl.h"

#include "slang-castable-util.h"

namespace Slang {

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
    add(CastableUtil::getCastable(unk));
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
