// slang-artifact.h
#ifndef SLANG_ARTIFACT_H
#define SLANG_ARTIFACT_H

#include "../core/slang-basic.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

enum class ArtifactKind : uint8_t
{
    None,                       ///< There is no container

    Unknown,                    ///< There is a container of unknown type

    Library,                    ///< Library of object code (typically made up multiple ObjectCode)
    ObjectCode,                 ///< Object code (for CPU typically .o or .obj file types)

    Executable,                 ///< Self contained such it can exectuted. On GPU this would be a kernel.
    SharedLibrary,              ///< Shared library/dll 
    Callable,                   ///< Callable directly (can mean there isn't a binary artifact)

    Text,                       ///< Text

    Container,                  ///< A container holding other things

    CountOf,
};

enum class ArtifactPayload : uint8_t
{
    None,           ///< There is no payload

    Unknown,        ///< Has payload but its unknown variety

    DXIL,
    DXBC,
    SPIRV,
    PTX,

    DXILAssembly,
    DXBCAssembly,
    SPIRVAssembly,
    PTXAssembly,

    HostCPU,        ///< The host CPU architecture

    SlangIR,        ///< Slang IR
    LLVMIR,         ///< LLVM IR

    SlangAST,       ///< Slang AST

    X86,
    X86_64,
    AARCH,
    AARCH64,

    HLSL,           ///< HLSL
    GLSL,           ///< GLSL
    CPP,            ///< C++
    C,              ///< C Language
    CUDA,           ///< CUDA
    Slang,          ///< Slang

    DebugInfo,      ///< Debug information 

    Diagnostics,    ///< Diagnostics

    Zip,            ///< It's a zip 

    CountOf,
};

enum class ArtifactStyle : uint8_t
{
    Unknown,                ///< Unknown

    Kernel,                 ///< Compiled as `GPU kernel` style.        
    Host,                   ///< Compiled in `host` style

    CountOf,
};

typedef uint8_t ArtifactFlags;
struct ArtifactFlag
{
    enum Enum : ArtifactFlags
    {
        // Don't currently have any flags
    };
};


/**
A value type to describe aspects of the contents of an Artifact.
**/
struct ArtifactDesc
{
public:
    typedef ArtifactDesc This;

    typedef ArtifactKind Kind;
    typedef ArtifactPayload Payload;
    typedef ArtifactStyle Style;
    typedef ArtifactFlags Flags;
   
    typedef uint32_t PackedBacking;
    enum class Packed : PackedBacking;
    
        /// Get in packed format
    inline Packed getPacked() const;

    bool operator==(const This& rhs) const { return kind == rhs.kind && payload == rhs.payload && style == rhs.style && flags == rhs.flags;  }
    bool operator!=(const This& rhs) const { return !(*this == rhs); }

        /// Given a code gen target, get the equivalent ArtifactDesc
    static This makeFromCompileTarget(SlangCompileTarget target);

        /// Construct from the elements
    static This make(Kind inKind, Payload inPayload, Style inStyle = Style::Unknown, Flags flags = 0) { return This{ inKind, inPayload, inStyle, flags }; }

        /// Construct from the packed format
    inline static This make(Packed inPacked);

    Kind kind;
    Payload payload;
    Style style;
    Flags flags;
};

// --------------------------------------------------------------------------
inline ArtifactDesc::Packed ArtifactDesc::getPacked() const
{
    typedef PackedBacking IntType;
    return Packed((IntType(kind) << 24) |
        (IntType(payload) << 16) |
        (IntType(style) << 8) |
        flags);
}

// --------------------------------------------------------------------------
inline /* static */ArtifactDesc ArtifactDesc::make(Packed inPacked)
{
    const PackedBacking packed = PackedBacking(inPacked);

    This r;
    r.kind = Kind(packed >> 24);
    r.payload = Payload(uint8_t(packed >> 16));
    r.style = Style(uint8_t(packed >> 8));
    r.flags = uint8_t(packed);

    return r;
}


// Controls what items can be kept. 
enum class ArtifactKeep
{
    No,         ///< Don't keep the item
    Yes,        ///< Yes keep the final item
    All,        ///< Keep the final item and any intermediataries
};

/// True if can keep an intermediate item
SLANG_INLINE  bool canKeepIntermediate(ArtifactKeep keep) { return keep == ArtifactKeep::All; }
/// True if can keep
SLANG_INLINE bool canKeep(ArtifactKeep keep) { return Index(keep) >= Index(ArtifactKeep::Yes); }
/// Returns the keep type for an intermediate
SLANG_INLINE ArtifactKeep getIntermediateKeep(ArtifactKeep keep) { return (keep == ArtifactKeep::All) ? ArtifactKeep::All : ArtifactKeep::No; }

enum ArtifactPathType
{
    None,
    Temporary,                  ///< Is a temporary file
    Existing,                   ///< Is an existing file
};

/* The IArtifactInstance interface represents a single instance of a type that can be part of an artifact. It's special in so far 
as 

* IArtifactInstance can be queried for it's underlying object class
* Can optionally serialize into a blob
*/
class IArtifactInstance : public ISlangUnknown
{
    SLANG_COM_INTERFACE(0x311457a8, 0x1796, 0x4ebb, { 0x9a, 0xfc, 0x46, 0xa5, 0x44, 0xc7, 0x6e, 0xa9 })

        /// Convert the instance into a serializable blob. 
        /// Returns SLANG_E_NOT_IMPLEMENTED if an implementation doesn't implement
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL writeToBlob(ISlangBlob** blob) = 0;

        /// Queries for the backing object type. The type is represented by a guid. 
        /// If the object doesn't derive from the type guid the function returns nullptr. 
        /// Unlike the analagous queryInterface method the ref count remains unchanged. 
        /// NOTE! 
        /// Whilst this method *could) be used across an ABI boundary (whereas using something like dynamic_cast would not),
        /// it is generally dangerous to do so.
    virtual SLANG_NO_THROW void* SLANG_MCALL queryObject(const Guid& classGuid) = 0;
};

/* The IArtifact interface is designed to represent some Artifact of compilation. It could be input to or output from a compilation.

An abstraction is desirable here, because depending on the compiler the artifact/s could be

* A file on the file system
* A blob
* Multiple files
* Some other (perhaps multiple) in memory representations 
* A name 

The artifact uses the Blob as the canonical in memory representation. 

Some downstream compilers require the artifact to be available as a file system file, or to produce
artifacts that are files. The IArtifact type allows to abstract away this difference, including the
ability to turn an in memory representation into a temporary file on the file system. 

The mechanism also allows for 'Containers' which allow for Artifacts to contain other Artifacts (amongst other things).
Those artifacts may be other files. For example a downstream compilation that produces results as well as temporary
files could be a Container containing artifacts for

* Diagnostics
* Temporary files (of known and unknown types)
* Files that contain known types
* Callable interface (an ISlangSharedLibrary)

Each one of these additions is an 'Element'. An Element is an interface pointer and a Desc that describes what the 
inteface represents. Having the associated desc provides more detail about what the interface pointer actually is
without having to make the interface know what it is being used for. This allows an interface to be used in multiple 
ways - for example the ISlangBlob interface could be used to represent some text, or a compiled kernel. 

A more long term goals would be to

* Make Diagnostics into an interface (such it can be added to a Artifact result)
* Use Artifact and related types for downstream compiler
*/
class IArtifact : public ISlangUnknown
{
public:
    SLANG_COM_INTERFACE(0x57375e20, 0xbed, 0x42b6, { 0x9f, 0x5e, 0x59, 0x4f, 0x6, 0x2b, 0xe6, 0x90 })

    typedef ArtifactDesc Desc;

    typedef ArtifactKind Kind;
    typedef ArtifactPayload Payload;
    typedef ArtifactStyle Style;
    typedef ArtifactFlags Flags;
    typedef ArtifactKeep Keep;
    typedef ArtifactPathType PathType;
    
        /// Get the Desc defining the contents of the artifact
    virtual SLANG_NO_THROW Desc SLANG_MCALL getDesc() = 0;

        /// Returns true if the artifact in principal exists
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() = 0;

        /// Load as a blob
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBlob(Keep keep, ISlangBlob** outBlob) = 0;
    
        /// Require artifact is available as a file.
        /// NOTE! May need to serialize and write as a temporary file.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFile(Keep keep) = 0;

        /// Require artifact is available in file-like scenarion.
        ///
        /// This is similar to requireFile, but for some special cases doesn't actually require a
        /// *explicit* path/file.
        ///
        /// For example when system libraries are specified - the library paths may be known to
        /// a downstream compiler (or the path is passed in explicitly), in that case only the
        /// artifact name needs to be correct.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFileLike(Keep keep) = 0;
    
        /// Finds an instance of that has the the interface guid
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL findElement(const Guid& guid) = 0;

        /// Find an element that derives from IArtifactInstance, and which queryObject works with the classGuid
    virtual SLANG_NO_THROW void* SLANG_MCALL findElementObject(const Guid& classGuid) = 0;

        /// Add items
    virtual SLANG_NO_THROW void SLANG_MCALL setPath(PathType pathType, const char* filePath) = 0;

        /// Set the blob representing the contents of the asset
    virtual SLANG_NO_THROW void SLANG_MCALL setBlob(ISlangBlob* blob) = 0;

        /// Get the path type
    virtual SLANG_NO_THROW PathType SLANG_MCALL getPathType() = 0;
        /// Get the path
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() = 0;

        /// Get the name of the artifact. This can be empty.
    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() = 0;

        /// Add an interface
    virtual SLANG_NO_THROW void SLANG_MCALL addElement(const Desc& desc, ISlangUnknown* intf) = 0;
    
        /// Get the item at the index
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getElementAt(Index i) = 0;
        /// Get the desc associated with an element
    virtual SLANG_NO_THROW Desc SLANG_MCALL getElementDescAt(Index i) = 0;

        /// Remove the element at the specified index. 
    virtual SLANG_NO_THROW void SLANG_MCALL removeElementAt(Index i) = 0;

        /// Get the amount of elements
    virtual SLANG_NO_THROW Index SLANG_MCALL getElementCount() = 0;
};

/*
Discussion:

It could make sense to remove the explicit variables of a ISlangBlob, and the file backing from this interface, as they could 
all be implemented as element types presumably deriving from IArtifactInstance. Doing so would mean how a 'file' is turned into
a blob is abstracted. 

It may be helpful to be able to add temporary files to the artifact (such that they will be deleted when the artifact goes out of 
scope). Using an implementation of the File backed IArtifactInstance, with a suitable desc would sort of work, but it breaks the idea 
that any IArtifactInstance *represents* the contents of Artifact that contains it. Of course there could be types *not* deriving 
from IArtifactInstance that handle temporary file existance. This is probably the simplest answer to the problem.

Another issue occurs around wanting to hold multiple kernels within a container. The problem here is that although through the desc
we can identify what target a kernel is for, there is no way of telling what stage it is for.

When discussing the idea of a shader cache, one idea was to use a ISlangFileSystem (which could actually be a zip, or directory or in memory rep)
as the main structure. Within this it can contain kernels, and then a json manifest can describe what each of these actually are.

This all 'works', in that we can add an element of ISlangFileSystem with a desc of Container. Code that uses this can then go through the process 
of finding, and getting the blob, and find from the manifest what it means. That does sound a little tedious though. Perhaps we just have an interface
that handles this detail, such that we search for that first. That interface is just attached to the artifact as an element.
*/

/* Implementation of the IArtifact interface */
class Artifact : public ComObject, public IArtifact
{
public:
    
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    
        /// IArtifact impl
    virtual SLANG_NO_THROW Desc SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBlob(Keep keep, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFile(Keep keep) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFileLike(Keep keep) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL findElement(const Guid& guid) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void* SLANG_MCALL findElementObject(const Guid& classGuid) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL setPath(PathType pathType, const char* path) SLANG_OVERRIDE { _setPath(pathType, path); }
    virtual SLANG_NO_THROW void SLANG_MCALL setBlob(ISlangBlob* blob) SLANG_OVERRIDE { m_blob = blob; }
    virtual SLANG_NO_THROW PathType SLANG_MCALL getPathType() SLANG_OVERRIDE { return m_pathType; }
    virtual SLANG_NO_THROW const char* SLANG_MCALL getPath() SLANG_OVERRIDE { return m_path.getBuffer(); }
    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() SLANG_OVERRIDE { return m_name.getBuffer(); }
    virtual SLANG_NO_THROW void SLANG_MCALL addElement(const Desc& desc, ISlangUnknown* intf) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getElementAt(Index i) SLANG_OVERRIDE { return m_elements[i].value; }
    virtual SLANG_NO_THROW Desc SLANG_MCALL getElementDescAt(Index i) SLANG_OVERRIDE { return m_elements[i].desc; }
    virtual SLANG_NO_THROW void SLANG_MCALL removeElementAt(Index i) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Index SLANG_MCALL getElementCount() SLANG_OVERRIDE { return m_elements.getCount(); }

    /// Ctor
    Artifact(const Desc& desc, const String& name) :
        m_desc(desc),
        m_name(name)
    {}
    /// Dtor
    ~Artifact();

protected:
    void* getInterface(const Guid& uuid);

    void _setPath(PathType pathType, const String& path) { m_pathType = pathType; m_path = path; }

    struct Element
    {
        ArtifactDesc desc;
        ComPtr<ISlangUnknown> value;
    };

    Desc m_desc;                                ///< Description of the artifact
    String m_name;                              ///< Name of this artifact

    PathType m_pathType = PathType::None;       ///< What the path indicates
    String m_path;                              ///< The path 
    String m_temporaryLockPath;                 ///< The temporary lock path

    ComPtr<ISlangBlob> m_blob;                  ///< Blob to store result in memory

    List<Element> m_elements;                   ///< Associated elements
};

} // namespace Slang

#endif
