// slang-artifact-representation.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_H
#define SLANG_ARTIFACT_REPRESENTATION_H

#include "../core/slang-basic.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"
#include "../core/slang-destroyable.h"

namespace Slang
{

/* The IArtifactRepresentation interface represents a single representation that can be part of an artifact. It's special in so far 
as 

* IArtifactRepresentation can be queried for it's underlying object class
* Can optionally serialize into a blob
*/
class IArtifactRepresentation : public ICastable
{
    SLANG_COM_INTERFACE(0x311457a8, 0x1796, 0x4ebb, { 0x9a, 0xfc, 0x46, 0xa5, 0x44, 0xc7, 0x6e, 0xa9 })

        /// Convert the instance into a serializable blob. 
        /// Returns SLANG_E_NOT_IMPLEMENTED if an implementation doesn't implement
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** blob) = 0;

        /// Returns true if this representation exists and is available for use.
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() = 0;
};

/* A lock file */
class ILockFile : public ICastable
{
    SLANG_COM_INTERFACE(0x9177ea36, 0xa608, 0x4490, { 0x87, 0xf0, 0xf3, 0x93, 0x9, 0x7d, 0x36, 0xce })

        /// The path to a lock file. 
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;
        /// Optional, the file system it's on. If nullptr its on 'regular' OS file system.
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() = 0;

        /// Makes the lock file no longer owned. Doing so will make the path nullptr, and getFileSystem nullptr. 
    virtual SLANG_NO_THROW void SLANG_MCALL disown() = 0;
};

/* An implementation of ILockFile */
class LockFile : public ComBaseObject, public ILockFile
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

        // ICastable
        SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // ILockFile
    SLANG_NO_THROW const char* SLANG_MCALL getPath() SLANG_OVERRIDE;
    SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() SLANG_OVERRIDE;
    SLANG_NO_THROW void SLANG_MCALL disown() SLANG_OVERRIDE;

    /// Ctor
    LockFile(String path, ISlangMutableFileSystem* fileSystem) :
        m_path(path),
        m_fileSystem(fileSystem)
    {
    }

    ~LockFile();

protected:
    void* getInterface(const Guid& uuid);
    void* getObject(const Guid& uuid);

    ISlangMutableFileSystem* _getFileSystem();

    String m_path;
    ComPtr<ISlangMutableFileSystem> m_fileSystem;
};

/* 
A representation as a file. If it is a temporary file, it will likely disappear.
A file representation does not have to be a representation of a file on the file system.
That is indicated by getFileSystem returning nullptr. Then the path is the path on the *actual* OS file system.
This distinction is important as it is sometimes necessary to have an artifact stored on the OS file system
to be usable. */
class IFileArtifactRepresentation : public IArtifactRepresentation
{
public:
    enum class Kind
    {
        Reference,          ///< References a file on the file system
        Owned,              ///< File is *owned* by this instance and will be deleted when goes out of scope
        NameOnly,           ///< Typically used for items that can be found by the 'system'. The path is just a name, and cannot typically be loaded as a blob.
    };

        /// The the kind of file. 
    virtual SLANG_NO_THROW Kind SLANG_MCALL getKind() = 0;                     
        /// The path (on the file system)
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;              
        /// Get the lock file. Return nullptr if there is no lock file.
    virtual SLANG_NO_THROW ILockFile* SLANG_MCALL getLockFile() = 0;
        /// Optional, the file system it's on. If nullptr its on 'regular' OS file system.
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() = 0;       
};

/* Interface for types that are associated with an artifact, but aren't a representation, or are 
only part of a representation. */
class IArtifactAssociated : public ICastable
{
    SLANG_COM_INTERFACE(0xafc0e4db, 0x16d4, 0x4d7a, { 0x93, 0x5f, 0x3e, 0x47, 0x7a, 0x23, 0x2a, 0x7f })
};

/*
A representation of an artifact that is held in a file */
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
    virtual SLANG_NO_THROW ILockFile* SLANG_MCALL getLockFile() SLANG_OVERRIDE { return m_lockFile; }
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() SLANG_OVERRIDE { return m_fileSystem; }

    FileArtifactRepresentation(Kind kind, String path, ILockFile* lockFile, ISlangMutableFileSystem* fileSystem):
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

    ISlangMutableFileSystem* _getFileSystem();

    Kind m_kind;
    String m_path;
    ComPtr<ILockFile> m_lockFile;
    ComPtr<ISlangMutableFileSystem> m_fileSystem;
};

} // namespace Slang

#endif
