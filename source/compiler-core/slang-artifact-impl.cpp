// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"
#include "slang-artifact-desc-util.h"

#include "slang-artifact-handler-impl.h"

#include "slang-slice-allocator.h"

#include "../core/slang-castable.h"

namespace Slang {


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Artifact !!!!!!!!!!!!!!!!!!!!!!!!!!! */

IArtifactHandler* Artifact::_getHandler()
{
    return m_handler ? m_handler : DefaultArtifactHandler::getSingleton();
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
    for (auto rep : m_representations.getArrayView())
    {
        ICastable* castable = rep.get();
        if (auto artifactRep = as<IArtifactRepresentation>(castable))
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

SlangResult Artifact::requireFile(Keep keep, IOSFileArtifactRepresentation** outFileRep)
{
    auto handler = _getHandler();

    ComPtr<ICastable> castable;
    SLANG_RETURN_ON_FAIL(handler->getOrCreateRepresentation(this, IOSFileArtifactRepresentation::getTypeGuid(), keep, castable.writeRef()));

    auto fileRep = as<IOSFileArtifactRepresentation>(castable);
    fileRep->addRef();

    *outFileRep = fileRep;
    return SLANG_OK;
}

SlangResult Artifact::loadSharedLibrary(ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary)
{
    auto handler = _getHandler();

    ComPtr<ICastable> castable;
    SLANG_RETURN_ON_FAIL(handler->getOrCreateRepresentation(this, ISlangSharedLibrary::getTypeGuid(), keep, castable.writeRef()));
   
    auto lib = as<ISlangSharedLibrary>(castable);
    lib->addRef();

    *outSharedLibrary = lib;
    return SLANG_OK;
}

IArtifactHandler* Artifact::getHandler()
{
    return m_handler;
}

void Artifact::setHandler(IArtifactHandler* handler)
{
    m_handler = handler;
}

void Artifact::clear(IArtifact::ContainedKind kind)
{
    switch (kind)
    {
        case ContainedKind::Associated:     m_associated.clear(); break;
        case ContainedKind::Representation: m_representations.clear(); break;
        default: break;
    }
}

void Artifact::removeAt(ContainedKind kind, Index i)
{
    switch (kind)
    {
        case ContainedKind::Associated:     m_associated.removeAt(i); break;
        case ContainedKind::Representation: m_representations.removeAt(i); break;
        default: break;
    }
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
    m_associated.add(ComPtr<ICastable>(castable));
}

void* Artifact::find(ContainedKind kind, const Guid& guid)
{
    Slice<ICastable*> slice;

    switch (kind)
    {
        case ContainedKind::Associated:         slice = SliceUtil::asSlice(m_associated); break;
        case ContainedKind::Representation:     slice = SliceUtil::asSlice(m_representations); break; 
        case ContainedKind::Children:
        {
            const auto children = getChildren();
            slice = Slice<ICastable*>((ICastable*const*)children.data, children.count);
            break;
        }
    }

    for (const auto& cur : slice)
    {
        if (cur->castAs(guid))
        {
            return cur;
        }
    }
    return nullptr;
}

Slice<ICastable*> Artifact::getAssociated()
{
    auto view = m_associated.getArrayView();
    return Slice<ICastable*>(view.begin()->readRef(), view.getCount());
}

void Artifact::addRepresentation(ICastable* castable)
{
    SLANG_ASSERT(castable);
    if (m_representations.indexOf(castable) >= 0)
    {
        SLANG_ASSERT_FAILURE("Already have this representation");
        return;
    }
    m_representations.add(ComPtr<ICastable>(castable));
}

void Artifact::addRepresentationUnknown(ISlangUnknown* unk)
{
    SLANG_ASSERT(unk);

    {
        const auto view = makeConstArrayView((ISlangUnknown*const*)m_representations.getBuffer(), m_representations.getCount());
        if (view.indexOf(unk) >= 0)
        {
            SLANG_ASSERT_FAILURE("Already have this representation");
            return;
        }
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
        m_representations.add(ComPtr<ICastable>(adapter));
    }
}

Slice<ICastable*> Artifact::getRepresentations()
{
    const auto view = m_representations.getArrayView();
    return Slice<ICastable*>(view.begin()->readRef(), view.getCount());
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

void ArtifactContainer::clear(IArtifact::ContainedKind kind)
{
    switch (kind)
    {
        case ContainedKind::Associated:     m_associated.clear(); break;
        case ContainedKind::Representation: m_representations.clear(); break;
        case ContainedKind::Children:       m_children.clear(); break;
        default: break;
    }
}

void ArtifactContainer::removeAt(ContainedKind kind, Index i)
{
    switch (kind)
    {
        case ContainedKind::Associated:     m_associated.removeAt(i); break;
        case ContainedKind::Representation: m_representations.removeAt(i); break;
        case ContainedKind::Children:       m_children.removeAt(i); break;

        default: break;
    }
}

} // namespace Slang
