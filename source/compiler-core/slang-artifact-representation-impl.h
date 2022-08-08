// slang-artifact-representation-impl.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_IMPL_H
#define SLANG_ARTIFACT_REPRESENTATION_IMPL_H

#include "slang-artifact-representation.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

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

} // namespace Slang

#endif
