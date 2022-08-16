// slang-artifact-representation-impl.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_IMPL_H
#define SLANG_ARTIFACT_REPRESENTATION_IMPL_H

#include "slang-artifact-representation.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"
#include "../core/slang-memory-arena.h"

namespace Slang
{

/* A representation of an artifact that is held in a file */
class FileArtifactRepresentation : public ComBaseObject, public IFileArtifactRepresentation
{
public:
    typedef IFileArtifactRepresentation::Kind Kind;

    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // IArtifactRepresentation
    SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** blob) SLANG_OVERRIDE;
    SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;

    // IFileArtifactRepresentation
    virtual SLANG_NO_THROW Kind SLANG_MCALL getKind() SLANG_OVERRIDE { return m_kind; }
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() SLANG_OVERRIDE { return m_path.getBuffer(); }
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() SLANG_OVERRIDE { return m_fileSystem; }
    virtual SLANG_NO_THROW void SLANG_MCALL disown() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IFileArtifactRepresentation* SLANG_MCALL getLockFile() SLANG_OVERRIDE { return m_lockFile; }

    FileArtifactRepresentation(Kind kind, String path, IFileArtifactRepresentation* lockFile, ISlangMutableFileSystem* fileSystem):
        m_kind(kind),
        m_path(path),
        m_lockFile(lockFile),
        m_fileSystem(fileSystem)
    {
    }

    ~FileArtifactRepresentation();

protected:
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

        /// True if the file is owned
    bool _isOwned() const { return Index(m_kind) >= Index(Kind::Owned); }

    ISlangMutableFileSystem* _getFileSystem();

    Kind m_kind;
    String m_path;
    ComPtr<IFileArtifactRepresentation> m_lockFile;
    ComPtr<ISlangMutableFileSystem> m_fileSystem;
};

class DiagnosticsArtifactRepresentation : public ComBaseObject, public IDiagnosticsArtifactRepresentation
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;
    // IArtifactRepresentation
    SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** blob) SLANG_OVERRIDE;
    SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;
    // IDiagnosticArtifactRepresentation
    SLANG_NO_THROW virtual const Diagnostic* SLANG_MCALL getAt(Index i) SLANG_OVERRIDE { return &m_diagnostics[i]; }
    SLANG_NO_THROW virtual Count SLANG_MCALL getCount() SLANG_OVERRIDE { return m_diagnostics.getCount(); }
    SLANG_NO_THROW virtual void SLANG_MCALL add(const Diagnostic& diagnostic) SLANG_OVERRIDE; 
    SLANG_NO_THROW virtual void SLANG_MCALL removeAt(Index i) SLANG_OVERRIDE { m_diagnostics.removeAt(i); }
    SLANG_NO_THROW virtual SlangResult SLANG_MCALL getResult() SLANG_OVERRIDE { return m_result; }
    SLANG_NO_THROW virtual void SLANG_MCALL setResult(SlangResult res) SLANG_OVERRIDE { m_result = res; }

    DiagnosticsArtifactRepresentation():
        m_arena(1024)
    {
    }

protected:
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    ZeroTerminatedCharSlice _allocateSlice(const Slice<char>& in);

    // We could consider storing paths, codes in StringSlicePool, but for now we just allocate all 'string type things'
    // in the arena.
    MemoryArena m_arena;

    List<Diagnostic> m_diagnostics;
    SlangResult m_result = SLANG_OK;
    
    ZeroTerminatedCharSlice m_raw;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! PostEmitMetadataArtifactRepresentation !!!!!!!!!!!!!!!!!!!!!!!!!! */

struct ShaderBindingRange
{
    slang::ParameterCategory category = slang::ParameterCategory::None;
    UInt spaceIndex = 0;
    UInt registerIndex = 0;
    UInt registerCount = 0; // 0 for unsized

    bool isInfinite() const
    {
        return registerCount == 0;
    }

    bool containsBinding(slang::ParameterCategory _category, UInt _spaceIndex, UInt _registerIndex) const
    {
        return category == _category
            && spaceIndex == _spaceIndex
            && registerIndex <= _registerIndex
            && (isInfinite() || registerCount + registerIndex > _registerIndex);
    }

    bool intersectsWith(const ShaderBindingRange& other) const
    {
        if (category != other.category || spaceIndex != other.spaceIndex)
            return false;

        const bool leftIntersection = (registerIndex < other.registerIndex + other.registerCount) || other.isInfinite();
        const bool rightIntersection = (other.registerIndex < registerIndex + registerCount) || isInfinite();

        return leftIntersection && rightIntersection;
    }

    bool adjacentTo(const ShaderBindingRange& other) const
    {
        if (category != other.category || spaceIndex != other.spaceIndex)
            return false;

        const bool leftIntersection = (registerIndex <= other.registerIndex + other.registerCount) || other.isInfinite();
        const bool rightIntersection = (other.registerIndex <= registerIndex + registerCount) || isInfinite();

        return leftIntersection && rightIntersection;
    }

    void mergeWith(const ShaderBindingRange other)
    {
        UInt newRegisterIndex = Math::Min(registerIndex, other.registerIndex);

        if (other.isInfinite())
            registerCount = 0;
        else if (!isInfinite())
            registerCount = Math::Max(registerIndex + registerCount, other.registerIndex + other.registerCount) - newRegisterIndex;

        registerIndex = newRegisterIndex;
    }

    static bool isUsageTracked(slang::ParameterCategory category)
    {
        switch (category)
        {
        case slang::ConstantBuffer:
        case slang::ShaderResource:
        case slang::UnorderedAccess:
        case slang::SamplerState:
            return true;
        default:
            return false;
        }
    }
};

struct PostEmitMetadata 
{
    List<ShaderBindingRange> usedBindings;
};

class PostEmitMetadataArtifactRepresentation : public ComBaseObject, public IPostEmitMetadataArtifactRepresentation
{
public:
    SLANG_CLASS_GUID(0x6f82509f, 0xe48b, 0x4b83, { 0xa3, 0x84, 0x5d, 0x70, 0x83, 0x19, 0x83, 0xcc })

    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;
    // IArtifactRepresentation
    SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** outBlob) SLANG_OVERRIDE { SLANG_UNUSED(outBlob); return SLANG_E_NOT_AVAILABLE; }
    SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE { return true; }
    // IPostEmitMetadataArtifactRepresentation
    SLANG_NO_THROW virtual Slice<ShaderBindingRange> SLANG_MCALL getBindingRanges() SLANG_OVERRIDE;
    
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    PostEmitMetadata m_metadata;
};

/* This allows wrapping any object to be an artifact representation. 

NOTE! Only allows casting from a single guid. Passing a RefObject across an ABI bounday remains risky!
*/
class ObjectArtifactRepresentation : public ComBaseObject, public IArtifactRepresentation
{
public:
    SLANG_CLASS_GUID(0xb9d5af57, 0x725b, 0x45f8, { 0xac, 0xed, 0x18, 0xf4, 0xa8, 0x4b, 0xf4, 0x73 })

    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;
    // IArtifactRepresentation
    SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** outBlob) SLANG_OVERRIDE { SLANG_UNUSED(outBlob); return SLANG_E_NOT_AVAILABLE; }
    SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE { return m_object; }

    ObjectArtifactRepresentation(const Guid& typeGuid, RefObject* obj):
        m_typeGuid(typeGuid), 
        m_object(obj)
    {
    }

    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);
 
    Guid m_typeGuid;                ///< Will return m_object if a cast to m_typeGuid is given
    RefPtr<RefObject> m_object;     ///< The object
};

} // namespace Slang

#endif
