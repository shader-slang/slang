// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"

namespace Slang {

/*
If we use LazyCastableList for Items, it means we'll have to use the
UnknownCastableAdapter for ISharedLibrary, IBlob etc. 

That means when we look for an item in the list, we will always do a query interface on those types, 
although it will always fail (so no atomic ref count). 

That doesn't seem wholey unreasonable. 

Note that we *can* derive from ICastable for *our* implementations of ISlangBlob, ISharedLibrary. So the 
kludge is only needed for types that really do require adaption. For Blob we'll require multiple interface inheritance.
*/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LazyCastableList !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void LazyCastableList::removeAt(Index index)
{
    SLANG_ASSERT(castable);
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
        if (castable)
        {
            if (auto ptr = castable->castAs(guid))
            {
                return ptr;
            }
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactList !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ArtifactList::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifactList::getTypeGuid())
    {
        return static_cast<IArtifactList*>(this);
    }
    return nullptr;
}

void* ArtifactList::getObject(const Guid& guid)
{
    // For now we can't cast to an object
    SLANG_UNUSED(guid);
    return nullptr;
}

void* ArtifactList::castAs(const Guid& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void ArtifactList::add(IArtifact* artifact)
{
    // Must be set
    SLANG_ASSERT(artifact);
    // Can't already be in the list
    SLANG_ASSERT(m_artifacts.indexOf(artifact) < 0);
    // Can't have another owner
    SLANG_ASSERT(artifact->getParent() == nullptr);

    // Set the parent
    artifact->setParent(m_parent);

    // Add
    m_artifacts.add(ComPtr<IArtifact>(artifact));
}

void ArtifactList::removeAt(Index index) 
{
   IArtifact* artifact = m_artifacts[index];
   artifact->setParent(nullptr);
   m_artifacts.removeAt(index); 
}

void ArtifactList::clear()
{
    _setParent(nullptr);
    m_artifacts.clear();
}

void ArtifactList::_setParent(IArtifact* parent)
{
    if (m_parent == parent)
    {
        return;
    }

    for (IArtifact* artifact : m_artifacts)
    {
        artifact->setParent(artifact);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* Artifact::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || uuid == IArtifact::getTypeGuid())
    {
        return static_cast<IArtifact*>(this);
    }
    return nullptr;
}

bool Artifact::exists()
{
    for (ISlangUnknown* item : m_items)
    {
        ComPtr<IArtifactRepresentation> rep;
        if (SLANG_SUCCEEDED(item->queryInterface(IArtifactRepresentation::getTypeGuid(), (void**)rep.writeRef())) && rep)
        {
            // It is a rep and it exists
            if (rep->exists())
            {
                return true;
            }
            // Might be another rep that exists
            continue;
        }

        // Must be not derived from IArtifactRepresentation, so we assume it's existance is it is a representation
        // so it exists
        return true;
    }

    return false;
}

void Artifact::addItem(ISlangUnknown* intf) 
{ 
    SLANG_ASSERT(intf);
    // Can't already be in there
    SLANG_ASSERT(m_items.indexOf(intf) < 0);
    // Add it
    m_items.add(ComPtr<ISlangUnknown>(intf));
}

void Artifact::removeItemAt(Index i)
{
    m_items.removeAt(i);
}

void* Artifact::findItemInterface(const Guid& guid)
{
    for (ISlangUnknown* intf : m_items)
    {
        ISlangUnknown* cast = nullptr;
        if (SLANG_SUCCEEDED(intf->queryInterface(guid, (void**)&cast)) && cast)
        {
            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            cast->release();
            return cast;
        }
    }
    return nullptr;
}

void* Artifact::findItemObject(const Guid& classGuid)
{
    for (ISlangUnknown* intf : m_items)
    {
        ComPtr<ICastable> castable;
        if (SLANG_SUCCEEDED(intf->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            void* obj = castable->castAs(classGuid);

            // NOTE! This assumes we *DONT* need to ref count to keep an interface in scope
            // (as strict COM requires so as to allow on demand interfaces).
            
            // If could cast return the result
            if (obj)
            {
                return obj;
            }
        }
    }
    return nullptr;
}

SlangResult Artifact::requireFile(Keep keep, IFileArtifactRepresentation** outFileRep)
{
    auto util = ArtifactUtilImpl::getSingleton();
    return util->requireFileDefaultImpl(this, keep, outFileRep);
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    // If we have a blob just return it
    if (auto blob = findItem<ISlangBlob>(this))
    {
        blob->addRef();
        *outBlob = blob;
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Look for a representation that we can serialize into a blob
    for (ISlangUnknown* intf : m_items)
    {
        ComPtr<IArtifactRepresentation> rep;
        if (SLANG_SUCCEEDED(intf->queryInterface(IArtifactRepresentation::getTypeGuid(), (void**)rep.writeRef())) && rep)
        {
            SlangResult res = rep->writeToBlob(blob.writeRef());
            if (SLANG_SUCCEEDED(res) && blob)
            {
                break;
            }
        }
    }
     
    // Wasn't able to construct
    if (!blob)
    {
        return SLANG_E_NOT_FOUND;
    }

    // Put in cache 
    if (canKeep(keep))
    {
        addItem(blob);
    }

    *outBlob = blob.detach();
    return SLANG_OK;
}

void Artifact::addAssociated(ICastable* castable)
{
    SLANG_ASSERT(castable);
    m_associated.add(castable);
}
 
void* Artifact::findAssociated(const Guid& guid)
{
    return m_associated.find(guid);
}

ICastableList* Artifact::getAssociated()
{
    return m_associated.requireList();
}

} // namespace Slang
