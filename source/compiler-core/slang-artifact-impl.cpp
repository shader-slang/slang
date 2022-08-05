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
    for (auto rep : m_representations.getView())
    {
        if (auto artifactRep = as<IArtifactRepresentation>(rep))
        {
            // It is an artifact rep and it exists, we are done
            if (artifactRep->exists())
            {
                return true;
            }
        }
        else
        {
            // If it's *not* IArtifactRepresentation derived, it's existance *is* a representation
            return true;
        }
    }

    return false;
}

SlangResult Artifact::requireFile(Keep keep, IFileArtifactRepresentation** outFileRep)
{
    auto util = ArtifactUtilImpl::getSingleton();
    return util->requireFileDefaultImpl(this, keep, outFileRep);
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    // If we have a blob just return it
    if (auto blob = (ISlangBlob*)findRepresentation(ISlangBlob::getTypeGuid()))
    {
        blob->addRef();
        *outBlob = blob;
        return SLANG_OK;
    }

    ComPtr<ISlangBlob> blob;

    // Look for a representation that we can serialize into a blob
    for (auto rep : m_representations.getView())
    {
        if (auto artifactRep = as<IArtifactRepresentation>(rep))
        {
            SlangResult res = artifactRep->writeToBlob(blob.writeRef());
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
        addRepresentationUnknown(blob);
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

void Artifact::addRepresentation(IArtifactRepresentation* rep)
{
    SLANG_ASSERT(rep);
    if (m_representations.indexOf(rep) >= 0)
    {
        SLANG_ASSERT_FAILURE("Already have this representation");
        return;
    }
    m_representations.add(rep);
}

void Artifact::addRepresentationUnknown(ISlangUnknown* unk)
{
    SLANG_ASSERT(unk);
    if (m_representations.indexOfUnknown(unk) >= 0)
    {
        SLANG_ASSERT_FAILURE("Already have this representation");
        return;
    }

    ComPtr<ICastable> castable;
    if (SLANG_SUCCEEDED(unk->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
    {
        if (m_representations.indexOf(castable) >= 0)
        {
            SLANG_ASSERT_FAILURE("Already have this representation");
            return;
        }
        m_representations.add(castable);
    }
    else
    {
        UnknownCastableAdapter* adapter = new UnknownCastableAdapter(unk);
        m_representations.add(adapter);
    }
}

void* Artifact::findRepresentation(const Guid& guid)
{
    return m_representations.find(guid);
}

ICastableList* Artifact::getRepresentations()
{
    return m_representations.requireList();
}

} // namespace Slang
