// slang-artifact-representation.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_H
#define SLANG_ARTIFACT_REPRESENTATION_H

#include "slang-artifact.h"

namespace Slang
{

/* 
A representation as a file. 

A file representation does not have to be a file on the OS file system, and might specify a file 
contained in a virtual file system represented by ISlangMutableFileSystem.

If `getFileSystem()` returns nullptr, then the path does specify a file on the regular OS file system otherwise
the path specifies the file contained in the virtual file system.

This distinction is important as it is sometimes necessary to have an artifact stored on the OS file system
to be usable. */
class IFileArtifactRepresentation : public IArtifactRepresentation
{
public:
    SLANG_COM_INTERFACE(0xc7d7d3a4, 0x8683, 0x44b5, { 0x87, 0x96, 0xdf, 0xba, 0x9b, 0xc3, 0xf1, 0x7b });

    /* Determines ownership and other characteristics of the 'file' */
    enum class Kind
    {
        Reference,          ///< References a file on the file system
        NameOnly,           ///< Typically used for items that can be found by the 'system'. The path is just a name, and cannot typically be loaded as a blob.
        Owned,              ///< File is *owned* by this instance and will be deleted when goes out of scope
        Lock,               ///< An owned type, indicates potentially in part may only exist to 'lock' a path for a temporary file. Other files might exists based on the 'lock' path.
        CountOf,
    };
    
        /// The the kind of file. 
    virtual SLANG_NO_THROW Kind SLANG_MCALL getKind() = 0;                     
        /// The path (on the file system)
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;              
        /// Optional, the file system it's on. If nullptr its on the 'regular' OS file system.
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() = 0;       
        /// Makes the file no longer owned. Only applicable for Owned/Lock and they will become 'Reference'
    virtual SLANG_NO_THROW void SLANG_MCALL disown() = 0;
        /// Gets the 'lock file' if any associated with this file. Returns nullptr if there isn't one. 
        /// If this file is based on a 'lock file', the lock file must stay in scope at least as long as this does.
    virtual SLANG_NO_THROW IFileArtifactRepresentation* SLANG_MCALL getLockFile() = 0;
};

} // namespace Slang

#endif
