// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"
#include "slang-artifact-desc-util.h"

#include "../core/slang-castable-list-impl.h"

namespace Slang {


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* Artifact::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* Artifact::getInterface(const Guid& uuid)
{
    if (uuid == ISlangUnknown::getTypeGuid() || 
        uuid == ICastable::getTypeGuid() ||
        uuid == IArtifact::getTypeGuid())
    {
        return static_cast<IArtifact*>(this);
    }
    return nullptr;
}

void* Artifact::getObject(const Guid& uuid)
{
    SLANG_UNUSED(uuid);
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

SlangResult Artifact::requireFile(Keep keep, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outFileRep)
{
    auto util = ArtifactUtilImpl::getSingleton();
    return util->requireFileDefaultImpl(this, keep, fileSystem, outFileRep);
}

SlangResult Artifact::loadSharedLibrary(ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary)
{
    auto util = ArtifactUtilImpl::getSingleton();
    return util->loadSharedLibraryDefaultImpl(this, keep, outSharedLibrary);
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


ICastable* Artifact::findAssociatedWithPredicate(ICastableList::FindFunc findFunc, void* data)
{
    return m_associated.findWithPredicate(findFunc, data);
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

ICastable* Artifact::findRepresentationWithPredicate(ICastableList::FindFunc findFunc, void* data)
{
    return m_representations.findWithPredicate(findFunc, data);
}

ICastableList* Artifact::getRepresentations()
{
    return m_representations.requireList();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ArtifactContainer !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ArtifactContainer::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ICastable::getTypeGuid() ||
        guid == IArtifact::getTypeGuid() ||
        guid == IArtifactContainer::getTypeGuid())
    {
        return static_cast<IArtifactContainer*>(this);
    }
    return nullptr;
}

void* ArtifactContainer::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* ArtifactContainer::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void ArtifactContainer::setChildren(IArtifact** children, Count count)
{
    m_expandResult = SLANG_OK;

    m_children.clearAndDeallocate();
    m_children.setCount(count);

    ComPtr<IArtifact>* dst = m_children.getBuffer();
    for (Index i = 0; i < count; ++i)
    {
        dst[i] = children[i];
    }
}

SlangResult ArtifactContainer::expandChildren()
{
    if (m_expandResult == SLANG_E_UNINITIALIZED)
    {
        auto util = ArtifactUtilImpl::getSingleton();
        m_expandResult = util->expandChildrenDefaultImpl(this);

        SLANG_ASSERT(m_expandResult != SLANG_E_UNINITIALIZED);
    }
    return m_expandResult;
}

Slice<IArtifact*> ArtifactContainer::getChildren()
{
    _requireChildren();
    return Slice<IArtifact*>((IArtifact**)m_children.getBuffer(), m_children.getCount());
}

void ArtifactContainer::addChild(IArtifact* artifact)
{
    SLANG_ASSERT(artifact);
    SLANG_ASSERT(m_children.indexOf(artifact) < 0);
    m_children.add(ComPtr<IArtifact>(artifact));
}

void ArtifactContainer::removeChildAt(Index index)
{
    m_children.removeAt(index);
}

void ArtifactContainer::clearChildren()
{
    m_children.clearAndDeallocate();
}
IArtifact* ArtifactContainer::findChildByDesc(const ArtifactDesc& desc)
{
    for (IArtifact* artifact : m_children)
    {
        if (artifact->getDesc() == desc)
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findChildByDerivedDesc(const ArtifactDesc& desc)
{
    for (IArtifact* artifact : m_children)
    {
        const ArtifactDesc artifactDesc = artifact->getDesc();
        // TODO(JS): Currently this ignores flags in desc. That may or may not be right 
        // long term.
        if (isDerivedFrom(artifactDesc.kind, desc.kind) &&
            isDerivedFrom(artifactDesc.payload, desc.payload) &&
            isDerivedFrom(artifactDesc.style, desc.style))
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findChildByName(const char* name)
{
    for (IArtifact* artifact : m_children)
    {
        const char* artifactName = artifact->getName();

        if (artifactName == name ||
            ::strcmp(artifactName, name) == 0)
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findChildByPredicate(FindFunc func, void* data)
{
    for (IArtifact* artifact : m_children)
    {
        if (func(artifact, data))
        {
            return artifact;
        }
    }
    return nullptr;
}

} // namespace Slang
