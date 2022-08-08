// slang-artifact-representation.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_H
#define SLANG_ARTIFACT_REPRESENTATION_H

#include "slang-artifact.h"

namespace Slang
{

/* 
A representation as a file. If it is a temporary file, it will likely disappear.
A file representation does not have to be a representation of a file on the file system.
That is indicated by getFileSystem returning nullptr. Then the path is the path on the *actual* OS file system.
This distinction is important as it is sometimes necessary to have an artifact stored on the OS file system
to be usable. */
class IFileArtifactRepresentation : public IArtifactRepresentation
{
public:
        // NOTE! 
    enum class Kind
    {
        Reference,          ///< References a file on the file system
        NameOnly,           ///< Typically used for items that can be found by the 'system'. The path is just a name, and cannot typically be loaded as a blob.
        Owned,              ///< File is *owned* by this instance and will be deleted when goes out of scope
        Lock,               ///< An owned type, indicates potentially in part may only exist to 'lock' a path for a temporary file
        CountOf,
    };
    
        /// The the kind of file. 
    virtual SLANG_NO_THROW Kind SLANG_MCALL getKind() = 0;                     
        /// The path (on the file system)
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;              
        /// Optional, the file system it's on. If nullptr its on 'regular' OS file system.
    virtual SLANG_NO_THROW ISlangMutableFileSystem* SLANG_MCALL getFileSystem() = 0;       
        /// Makes the file no longer owned. Only applicable for Owned/Lock and they will become 'Reference'
    virtual SLANG_NO_THROW void SLANG_MCALL disown() = 0;
        /// Gets the 'lock file' if any associated with this file. Returns nullptr if there isn't one.
    virtual SLANG_NO_THROW IFileArtifactRepresentation* SLANG_MCALL getLockFile() = 0;
};

} // namespace Slang

#endif
