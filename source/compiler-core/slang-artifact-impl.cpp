// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"
#include "slang-artifact-desc-util.h"

#include "../core/slang-castable-list-impl.h"

namespace Slang {


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

IArtifactHandler* Artifact::_getHandler()
{
    // TODO(JS): For now we just use the default handler, but in the future this should probably be a member
    return DefaultArtifactHandler::getSingleton();
}

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
    auto handler = _getHandler();
    return handler->getOrCreateFileRepresentation(this, keep, fileSystem, outFileRep);
}

SlangResult Artifact::loadSharedLibrary(ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary)
{
    auto handler = _getHandler();

    ComPtr<ICastable> castable;
    SLANG_RETURN_ON_FAIL(handler->getOrCreateRepresentation(this, ISlangSharedLibrary::getTypeGuid(), keep, castable.writeRef()));
   
    ISlangSharedLibrary* lib = as<ISlangSharedLibrary>(castable);
    lib->addRef();

    *outSharedLibrary = lib;
    return SLANG_OK;
}

SlangResult Artifact::getOrCreateRepresentation(const Guid& typeGuid, ArtifactKeep keep, ICastable** outCastable)
{
    auto handler = _getHandler();
    return handler->getOrCreateRepresentation(this, typeGuid, keep, outCastable);
}

SlangResult Artifact::loadBlob(Keep keep, ISlangBlob** outBlob)
{
    auto handler = _getHandler();

    ComPtr<ICastable> castable;
    SLANG_RETURN_ON_FAIL(handler->getOrCreateRepresentation(this, ISlangBlob::getTypeGuid(), keep, castable.writeRef()));

    ISlangBlob* blob = as<ISlangBlob>(castable);
    blob->addRef();

    *outBlob = blob;
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

Slice<ICastable*> Artifact::getRepresentations()
{
    const auto view = m_representations.getView();
    return Slice<ICastable*>(view.getBuffer(), view.getCount());
}

ICastableList* Artifact::getRepresentationList()
{
    return m_representations.requireList();
}

IArtifact* Artifact::findRecursivelyByDerivedDesc(const ArtifactDesc& from)
{
    if (ArtifactDescUtil::isDescDerivedFrom(m_desc, from))
    {
        return this;
    }
    return nullptr;
}

IArtifact* Artifact::findRecursivelyByPredicate(FindFunc func, void* data)
{
    if (func(this, data))
    {
        return this;
    }
    return nullptr;
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
    auto handler = _getHandler();
    return handler->expandChildren(this);
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

    _requireChildren();

    m_children.add(ComPtr<IArtifact>(artifact));
}

void ArtifactContainer::removeChildAt(Index index)
{
    _requireChildren();

    m_children.removeAt(index);
}

void ArtifactContainer::clearChildren()
{
    _requireChildren();

    m_children.clearAndDeallocate();
}

IArtifact* ArtifactContainer::findChildByDesc(const ArtifactDesc& desc)
{
    _requireChildren();

    for (IArtifact* artifact : m_children)
    {
        if (artifact->getDesc() == desc)
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findChildByDerivedDesc(const ArtifactDesc& from)
{
    _requireChildren();

    for (IArtifact* artifact : m_children)
    {
        if (ArtifactDescUtil::isDescDerivedFrom(artifact->getDesc(), from))
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findChildByName(const char* name)
{
    _requireChildren();

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
    _requireChildren();

    for (IArtifact* artifact : m_children)
    {
        if (func(artifact, data))
        {
            return artifact;
        }
    }
    return nullptr;
}

IArtifact* ArtifactContainer::findRecursivelyByDerivedDesc(const ArtifactDesc& from)
{
    if (auto artifact = Super::findRecursivelyByDerivedDesc(from))
    {
        return artifact;
    }

    _requireChildren();

    for (IArtifact* artifact : m_children)
    {
        if (IArtifact* found = artifact->findRecursivelyByDerivedDesc(from))
        {
            return found;
        }
    }

    return nullptr;
}

IArtifact* ArtifactContainer::findRecursivelyByPredicate(FindFunc func, void* data)
{
    if (func(this, data))
    {
        return this;
    }

    _requireChildren();

    for (IArtifact* artifact : m_children)
    {
        if (IArtifact* found = artifact->findRecursivelyByPredicate(func, data))
        {
            return found;
        }
    }

    return nullptr;
}

} // namespace Slang
