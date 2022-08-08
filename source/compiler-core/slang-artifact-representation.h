// slang-artifact-representation.h
#ifndef SLANG_ARTIFACT_REPRESENTATION_H
#define SLANG_ARTIFACT_REPRESENTATION_H

#include "slang-artifact.h"

namespace Slang
{

/* Simplest slice types. We can't use UnownedStringSlice etc, because they implement functionality in libraries, 
and we want to use these types in headers. 
If we wanted a C implementation it would be easy to use a macro to generate the functionality */

template <typename T>
struct Slice
{
    Slice():count(0), data(nullptr) {}
    Slice(const T* inData, Count inCount): 
        data(inData), 
        count(inCount) 
    {}

    const T* data;
    Count count;
};

struct ZeroTerminatedCharSlice : Slice<char>
{
    typedef Slice<char> Super;
    explicit ZeroTerminatedCharSlice(const char* in):Super(in, ::strlen(in)) {}
    ZeroTerminatedCharSlice(const char* in, Count inCount):Super(in, inCount) { SLANG_ASSERT(in[inCount] == 0); }
    ZeroTerminatedCharSlice():Super("", 0) {}
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
    SLANG_COM_INTERFACE(0xc7d7d3a4, 0x8683, 0x44b5, { 0x87, 0x96, 0xdf, 0xba, 0x9b, 0xc3, 0xf1, 0x7b });

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

class IDiagnosticsArtifactRepresentation : public IArtifactRepresentation
{
public:
    SLANG_COM_INTERFACE(0x91f9b857, 0xcd6b, 0x45ca, { 0x8e, 0x3, 0x8f, 0xa3, 0x3c, 0x5c, 0xf0, 0x1a });

    enum class Severity
    {
        Unknown,
        Info,
        Warning,
        Error,
        CountOf,
    };
    enum class Stage
    {
        Compile,
        Link,
    };

    struct Location
    {
        Int line = 0;                   ///< One indexed line number. 0 if not defined
        Int column = 0;                 ///< One indexed *character (not byte)* column number. 0 if not defined
    };

    struct Diagnostic
    {
        Severity severity = Severity::Unknown;          ///< The severity of error
        Stage stage = Stage::Compile;                   ///< The stage the error came from
        ZeroTerminatedCharSlice text;                   ///< The text of the error
        ZeroTerminatedCharSlice code;                   ///< The compiler specific error code
        ZeroTerminatedCharSlice filePath;               ///< The path the error originated from
        Location location;
    };

        /// Get the diagnostic at the index
    SLANG_NO_THROW virtual const Diagnostic* SLANG_MCALL getAt(Index i) = 0;
        /// Get the amount of diangostics
    SLANG_NO_THROW virtual Count SLANG_MCALL getCount() = 0;
        /// Add a diagnostic
    SLANG_NO_THROW virtual void SLANG_MCALL add(const Diagnostic& diagnostic) = 0;
        /// Remove the diagnostic at the index
    SLANG_NO_THROW virtual void SLANG_MCALL removeAt(Index i) = 0;

        /// Set the raw diagnostics
    SLANG_NO_THROW virtual void SLANG_MCALL setRaw(const Slice<char>& in) = 0;
        /// Get raw diagnostice
    SLANG_NO_THROW virtual ZeroTerminatedCharSlice SLANG_MCALL getRaw() = 0; 

        /// Get the result for a compilation
    SLANG_NO_THROW virtual SlangResult SLANG_MCALL getResult() = 0;
        /// Set the result
    SLANG_NO_THROW virtual void SLANG_MCALL setResult(SlangResult res) = 0;
};

struct ShaderBindingRange;

class IPostEmitMetadataArtifactRepresentation : public IArtifactRepresentation
{
public:
    SLANG_COM_INTERFACE(0x5d03bce9, 0xafb1, 0x4fc8, { 0xa4, 0x6f, 0x3c, 0xe0, 0x7b, 0x6, 0x1b, 0x1b });
    
        /// Get the binding ranges
    SLANG_NO_THROW virtual Slice<ShaderBindingRange> SLANG_MCALL getBindingRanges() = 0;
};


} // namespace Slang

#endif
