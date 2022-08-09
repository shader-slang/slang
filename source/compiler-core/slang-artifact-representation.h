// slang-artifact-representation.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_H
#define SLANG_ARTIFACT_REPRESENTATION_H

#include "slang-artifact.h"

namespace Slang
{

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

} // namespace Slang

#endif
