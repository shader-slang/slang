// slang-artifact-impl.h
#ifndef SLANG_ARTIFACT_IMPL_H
#define SLANG_ARTIFACT_IMPL_H

#include "slang-artifact.h"

#include "../core/slang-lazy-castable-list.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

/*
Discussion:

Another issue occurs around wanting to hold multiple kernels within a container. The problem here is that although through the desc
we can identify what target a kernel is for, there is no way of telling what stage it is for.

When discussing the idea of a shader cache, one idea was to use a ISlangFileSystem (which could actually be a zip, or directory or in memory rep)
as the main structure. Within this it can contain kernels, and then a json manifest can describe what each of these actually are.

This all 'works', in that we can add an element of ISlangFileSystem with a desc of Container. Code that uses this can then go through the process 
of finding, and getting the blob, and find from the manifest what it means. That does sound a little tedious though. Perhaps we just have an interface
that handles this detail, such that we search for that first. That interface is just attached to the artifact as an element.

Note: Implementation of the IArtifact interface. We derive from IArtifactContainer, such that we don't have the 
irritating multiple inheritance issue. */
class Artifact : public ComBaseObject, public IArtifactContainer
{
public:
    
    SLANG_COM_BASE_IUNKNOWN_ALL
    
    /// ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    /// IArtifact impl
    virtual SLANG_NO_THROW Desc SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBlob(Keep keep, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFile(Keep keep, ISlangMutableFileSystem* fileSystem, IFileArtifactRepresentation** outFileRep) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(ArtifactKeep keep, ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() SLANG_OVERRIDE { return m_name.getBuffer(); }
    virtual SLANG_NO_THROW void SLANG_MCALL setName(const char* name) SLANG_OVERRIDE { m_name = name; }

    virtual SLANG_NO_THROW void SLANG_MCALL addAssociated(ICastable* castable) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void* SLANG_MCALL findAssociated(const Guid& unk) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ICastableList* SLANG_MCALL getAssociated() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ICastable* SLANG_MCALL findAssociatedWithPredicate(ICastableList::FindFunc findFunc, void* data) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW void SLANG_MCALL addRepresentation(ICastable* castable) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addRepresentationUnknown(ISlangUnknown* rep) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void* SLANG_MCALL findRepresentation(const Guid& guid) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Slice<ICastable*> SLANG_MCALL getRepresentations() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ICastableList* SLANG_MCALL getRepresentationList() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ICastable* SLANG_MCALL findRepresentationWithPredicate(ICastableList::FindFunc findFunc, void* data) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getOrCreateRepresentation(const Guid& typeGuid, ArtifactKeep keep, ICastable** outCastable) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW IArtifactHandler* SLANG_MCALL getHandler() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setHandler(IArtifactHandler* handler) SLANG_OVERRIDE;

    virtual SLANG_NO_THROW Slice<IArtifact*> SLANG_MCALL getChildren() SLANG_OVERRIDE { return Slice<IArtifact*>(nullptr, 0); }

    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByDerivedDesc(FindStyle findStyle, const ArtifactDesc& desc) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByPredicate(FindStyle findStyle, FindFunc func, void* data) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByName(FindStyle findStyle, const char* name) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByDesc(FindStyle findStyle, const ArtifactDesc& desc) SLANG_OVERRIDE;

    // IArtifactCollection (Not implemented)
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getExpandChildrenResult() SLANG_OVERRIDE { SLANG_UNREACHABLE("Not implemented"); }
    virtual SLANG_NO_THROW void SLANG_MCALL setChildren(IArtifact** children, Count count) SLANG_OVERRIDE { SLANG_UNUSED(children); SLANG_UNUSED(count); SLANG_UNREACHABLE("Not implemented"); }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL expandChildren() SLANG_OVERRIDE { SLANG_UNREACHABLE("Not implemented"); }
    virtual SLANG_NO_THROW void SLANG_MCALL addChild(IArtifact* artifact) SLANG_OVERRIDE { SLANG_UNUSED(artifact); SLANG_UNREACHABLE("Not implemented"); }
    virtual SLANG_NO_THROW void SLANG_MCALL removeChildAt(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); SLANG_UNREACHABLE("Not implemented"); }
    virtual SLANG_NO_THROW void SLANG_MCALL clearChildren() SLANG_OVERRIDE { SLANG_UNREACHABLE("Not implemented"); }

    static ComPtr<IArtifact> create(const Desc& desc) { return ComPtr<IArtifact>(new Artifact(desc)); }
    static ComPtr<IArtifact> create(const Desc& desc, const UnownedStringSlice& name) { return ComPtr<IArtifact>(new Artifact(desc, name)); }
    
protected:

    /// Ctor
    Artifact(const Desc& desc, const UnownedStringSlice& name) :
        m_desc(desc),
        m_name(name),
        m_parent(nullptr)
    {}
    Artifact(const Desc& desc) :
        m_desc(desc),
        m_parent(nullptr)
    {}

    IArtifactHandler* _getHandler();

    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    Desc m_desc;                                ///< Description of the artifact
    IArtifact* m_parent;                        ///< Artifact this artifact belongs to

    String m_name;                              ///< Name of this artifact

    ComPtr<IArtifactHandler> m_handler;         ///< The handler. Can be nullptr and then default handler is used.

    LazyCastableList m_associated;              ///< Associated items
    LazyCastableList m_representations;         ///< Representations
};

class ArtifactContainer : public Artifact
{
public:
    typedef Artifact Super;
    SLANG_COM_BASE_IUNKNOWN_QUERY_INTERFACE

    /// ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    /// IArtifact
    virtual SLANG_NO_THROW Slice<IArtifact*> SLANG_MCALL getChildren() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByDerivedDesc(FindStyle findStyle, const ArtifactDesc& desc) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByPredicate(FindStyle findStyle, FindFunc func, void* data) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByName(FindStyle findStyle, const char* name) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL findArtifactByDesc(FindStyle findStyle, const ArtifactDesc& desc) SLANG_OVERRIDE;

    // IArtifactCollection
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getExpandChildrenResult() SLANG_OVERRIDE { return m_expandResult; }
    virtual SLANG_NO_THROW void SLANG_MCALL setChildren(IArtifact** children, Count count) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL expandChildren() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addChild(IArtifact* artifact) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL removeChildAt(Index index) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL clearChildren() SLANG_OVERRIDE;
    
    static ComPtr<IArtifactContainer> create(const Desc& desc) { return ComPtr<IArtifactContainer>(new ArtifactContainer(desc)); }
    static ComPtr<IArtifactContainer> create(const Desc& desc, const UnownedStringSlice& name) { return ComPtr<IArtifactContainer>(new ArtifactContainer(desc, name)); }

protected:
    /// Ctor
    ArtifactContainer(const Desc& desc, const UnownedStringSlice& name) :Super(desc, name) {}
    ArtifactContainer(const Desc& desc) : Super(desc) {}

    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);
    void _requireChildren()
    {
        if (m_expandResult == SLANG_E_UNINITIALIZED)
        {
            const auto res = expandChildren();
            SLANG_UNUSED(res);
            SLANG_ASSERT(SLANG_SUCCEEDED(res));
        }
    }

    SlangResult m_expandResult = SLANG_E_UNINITIALIZED;

    List<ComPtr<IArtifact>> m_children;
};

} // namespace Slang

#endif
