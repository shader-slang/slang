// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"

namespace Slang {

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
        ComPtr<ICastable> castable;

        if (SLANG_SUCCEEDED(item->queryInterface(ICastable::getTypeGuid(), (void**)castable.writeRef())) && castable)
        {
            auto rep = as<IArtifactRepresentation>(castable);
            if (rep)
            {
                // It is a rep and it exists
                if (rep->exists())
                {
                    return true;
                }
                continue;
            }
            // Associated types don't encapsulate an artifact representation, so don't signal existance
            if (as<IArtifactAssociated>(castable))
            {
                continue;
            }
        }
        
        // It can't be IArtifactRepresentation or IArtifactAssociated, so we assume means it exists
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

} // namespace Slang
