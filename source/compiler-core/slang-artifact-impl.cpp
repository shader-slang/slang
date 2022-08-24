// slang-artifact-impl.cpp
#include "slang-artifact-impl.h"

#include "slang-artifact-representation.h"

#include "slang-artifact-util.h"
#include "slang-artifact-desc-util.h"

#include "slang-artifact-handler-impl.h"

#include "../core/slang-castable-util.h"

namespace Slang {

static bool _checkSelf(IArtifact::FindStyle findStyle)
{
    return Index(findStyle) <= Index(IArtifact::FindStyle::SelfOrChildren);
}

static bool _checkChildren(IArtifact::FindStyle findStyle)
{
    return Index(findStyle) >= Index(IArtifact::FindStyle::SelfOrChildren);
}

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

IArtifactHandler* Artifact::getHandler()
{
    return m_handler;
}

void Artifact::setHandler(IArtifactHandler* handler)
{
    m_handler = handler;
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

IArtifact* Artifact::findArtifactByDerivedDesc(FindStyle findStyle, const ArtifactDesc& from)
{
    return (_checkSelf(findStyle) && ArtifactDescUtil::isDescDerivedFrom(m_desc, from)) ? this : nullptr;
}

IArtifact* Artifact::findArtifactByPredicate(FindStyle findStyle, FindFunc func, void* data)
{
    return (_checkSelf(findStyle) && func(this, data)) ? this : nullptr;
}

IArtifact* Artifact::findArtifactByName(FindStyle findStyle, const char* name)
{
    return (_checkSelf(findStyle) && m_name == name) ? this : nullptr;
}

IArtifact* Artifact::findArtifactByDesc(FindStyle findStyle, const ArtifactDesc& desc)
{
    return (_checkSelf(findStyle) && m_desc == desc) ? this : nullptr;
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

static bool _isDerivedDesc(IArtifact* artifact, void* data)
{
    const ArtifactDesc& from = *(const ArtifactDesc*)data;
    return ArtifactDescUtil::isDescDerivedFrom(artifact->getDesc(), from);
}

static bool _isDesc(IArtifact* artifact, void* data)
{
    const ArtifactDesc& desc = *(const ArtifactDesc*)data;
    return desc == artifact->getDesc();
}

static bool _isName(IArtifact* artifact, void* data)
{
    const char* name = (const char*)data;
    const char* artifactName = artifact->getName();
    if (artifactName == nullptr)
    {
        return false;
    }
    return ::strcmp(name, artifactName) == 0;
}


IArtifact* ArtifactContainer::findArtifactByDerivedDesc(FindStyle findStyle, const ArtifactDesc& from)
{
    return findArtifactByPredicate(findStyle, _isDerivedDesc, const_cast<ArtifactDesc*>(&from));
}

IArtifact* ArtifactContainer::findArtifactByName(FindStyle findStyle, const char* name)
{
    return findArtifactByPredicate(findStyle, _isName, const_cast<char*>(name));
}

IArtifact* ArtifactContainer::findArtifactByDesc(FindStyle findStyle, const ArtifactDesc& desc)
{
    return findArtifactByPredicate(findStyle, _isDesc, const_cast<ArtifactDesc*>(&desc));
}

IArtifact* ArtifactContainer::findArtifactByPredicate(FindStyle findStyle, FindFunc func, void* data)
{
    if (_checkSelf(findStyle) && func(this, data))
    {
        return this;
    }

    if (_checkChildren(findStyle))
    {
        auto children = getChildren();

        // First search the children
        for (auto child : children)
        {
            if (func(child, data))
            {
                return child;
            }
        }

        // Then the childrens recursively
        if (findStyle == FindStyle::Recursive ||
            findStyle == FindStyle::ChildrenRecursive)
        {
            for (auto child : children)
            {
                if (auto found = child->findArtifactByPredicate(FindStyle::ChildrenRecursive, func, data))
                {
                    return found;
                }
            }
        }
    }

    return nullptr;
}

} // namespace Slang
