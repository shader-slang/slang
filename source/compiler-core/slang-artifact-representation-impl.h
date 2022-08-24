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
    typedef FileArtifactRepresentation ThisType;

    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // IArtifactRepresentation
    SLANG_NO_THROW SlangResult SLANG_MCALL createRepresentation(const Guid& typeGuid, ICastable** outCastable) SLANG_OVERRIDE;
    SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;

    // IFileArtifactRepresentation
    virtual SLANG_NO_THROW Kind SLANG_MCALL getKind() SLANG_OVERRIDE { return m_kind; }
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() SLANG_OVERRIDE { return m_path.getBuffer(); }
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() SLANG_OVERRIDE { return m_fileSystem; }
    virtual SLANG_NO_THROW void SLANG_MCALL disown() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW IFileArtifactRepresentation* SLANG_MCALL getLockFile() SLANG_OVERRIDE { return m_lockFile; }

    FileArtifactRepresentation(Kind kind, const UnownedStringSlice& path, IFileArtifactRepresentation* lockFile, ISlangMutableFileSystem* fileSystem):
        m_kind(kind),
        m_lockFile(lockFile),
        m_path(path),
        m_fileSystem(fileSystem)
    {
    }

    ~FileArtifactRepresentation();

    static ComPtr<IFileArtifactRepresentation> create(Kind kind, const UnownedStringSlice& path, IFileArtifactRepresentation* lockFile, ISlangMutableFileSystem* fileSystem)
    {
        return ComPtr<IFileArtifactRepresentation>(new ThisType(kind, path, lockFile, fileSystem)); 
    }

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
    SLANG_NO_THROW SlangResult SLANG_MCALL createRepresentation(const Guid& guid, ICastable** outCastable) SLANG_OVERRIDE { SLANG_UNUSED(guid); SLANG_UNUSED(outCastable); return SLANG_E_NOT_AVAILABLE; }
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
