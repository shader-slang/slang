// slang-artifact-representation-impl.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_IMPL_H
#define SLANG_ARTIFACT_REPRESENTATION_IMPL_H

#include "slang-artifact-representation.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"
#include "../core/slang-memory-arena.h"

//#include "../core/slang-string-slice-pool.h"

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
    SLANG_NO_THROW virtual void SLANG_MCALL setRaw(const Slice<char>& in) { m_raw = _allocateSlice(in); }
    SLANG_NO_THROW virtual ZeroTerminatedCharSlice SLANG_MCALL getRaw() SLANG_OVERRIDE { return m_raw; }
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

} // namespace Slang

#endif
