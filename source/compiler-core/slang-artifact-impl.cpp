// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"

#include "../core/slang-castable-list-impl.h"

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

void Artifact::addRepresentation(ICastable* castable)
{
    SLANG_ASSERT(castable);
    if (m_representations.indexOf(castable) >= 0)
    {
        SLANG_ASSERT_FAILURE("Already have this representation");
        return;
    }
    m_representations.add(castable);
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

IArtifactList* Artifact::getChildren()
{
    // If it has already evaluated, return it.
    if (m_children)
    {
        return m_children;
    }

    auto util = ArtifactUtilImpl::getSingleton();
    util->getChildrenDefaultImpl(this, m_children.writeRef());

    return m_children;
}


} // namespace Slang
