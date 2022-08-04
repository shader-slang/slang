// slang-artifact.h
#ifndef SLANG_ARTIFACT_H
#define SLANG_ARTIFACT_H

#include "../core/slang-basic.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"
#include "../core/slang-destroyable.h"

namespace Slang
{

/* As a rule of thumb, if we can define some aspect in a hierarchy then we should do so at the highest level. 
If some aspect can apply to multiple items identically we move that to a separate enum. 

NOTE!
New Kinds must be added at the end. Values can be depreciated, or disabled
but never removed, without breaking binary compatability.

Any change requires a change to SLANG_ARTIFACT_KIND
*/
enum class ArtifactKind : uint8_t
{ 
    Invalid,                    ///< Invalid
    Base,                       ///< Base kind of all valid kinds

    None,                       ///< Doesn't contain anything
    Unknown,                    ///< Unknown

    Container,                  ///< Container like types
    Zip,                        ///< Zip container
    Riff,                       ///< Riff format

    Text,                       ///< Representation is text. Encoding is utf8, unless prefixed with 'encoding'.
    
    Source,                     ///< Source (Source type is in payload)
    Assembly,                   ///< Assembly (Type is in payload)
    HumanText,                  ///< Text for human consumption

    BinaryLike,                 ///< Kinds which are 'binary like' - can be executed, linked with and so forth. 
    
    ObjectCode,                 ///< Object file
    Library,                    ///< Library (collection of object code)
    Executable,                 ///< Executable
    SharedLibrary,              ///< Shared library - can be dynamically linked
    HostCallable,               ///< Code can be executed directly on the host

    DebugInfo,                   ///< Debugging information
    Diagnostics,                 ///< Diagnostics information

    CountOf,
};

/* Payload. 

SlangIR and LLVMIR can be GPU or CPU orientated, so put in own category.

NOTE!
New Payloads must be added at the end. Values can be depreciated, or disabled
but never removed, without breaking binary compatability.

Any change requires a change to SLANG_ARTIFACT_PAYLOAD
*/
enum class ArtifactPayload : uint8_t
{
    Invalid,        ///< Is invalid - indicates some kind of problem
    Base,           ///< The base of the hierarchy

    None,           ///< Doesn't have a payload
    Unknown,        ///< Unknown but probably valid
    
    Source,         ///< Source code
    
    C,              ///< C source
    Cpp,            ///< C++ source
    HLSL,           ///< HLSL source
    GLSL,           ///< GLSL source
    CUDA,           ///< CUDA source
    Metal,          ///< Metal source
    Slang,          ///< Slang source

    KernelLike,     ///< GPU Kernel like

    DXIL,           ///< DXIL 
    DXBC,           ///< DXBC
    SPIRV,          ///< SPIR-V
    PTX,            ///< PTX. NOTE! PTX is a text format, but is handable to CUDA API.
    MetalAIR,       ///< Metal AIR 
    CuBin,          ///< CUDA binary

    CPULike,        ///< CPU code
    
    UnknownCPU,     ///< CPU code for unknown/undetermined type
    X86,            ///< X86
    X86_64,         ///< X86_64
    Aarch,          ///< 32 bit arm
    Aarch64,        ///< Aarch64
    HostCPU,        ///< HostCPU
    UniversalCPU,   ///< CPU code for multiple CPU types 

    GeneralIR,      ///< General purpose IR representation (IR)

    SlangIR,        ///< Slang IR
    LLVMIR,         ///< LLVM IR

    AST,            ///< Abstract syntax tree (AST)

    SlangAST,       ///< Slang AST

    CountOf,
};

/* Style.

NOTE!
New Styles must be added at the end. Values can be depreciated, or disabled
but never removed, without breaking binary compatability.

Any change requires a change to SLANG_ARTIFACT_STYLE
*/
enum class ArtifactStyle : uint8_t
{
    Invalid,            ///< Invalid style (indicating an error)
    Base,
        
    Unknown,            ///< Unknown

    Kernel,             ///< Compiled as `GPU kernel` style.        
    Host,               ///< Compiled in `host` style

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

// Forward declare
class IFileArtifactRepresentation;

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
    
        /// Get the Desc defining the contents of the artifact
    virtual SLANG_NO_THROW Desc SLANG_MCALL getDesc() = 0;

        /// Get the artifact (if any) that this artifact belongs to
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL getParent() = 0;
        /// Set the parent that 'owns' this artifact. The parent is *not* reference counted (ie weak reference)
    virtual SLANG_NO_THROW void SLANG_MCALL setParent(IArtifact* parent) = 0;

        /// Returns true if the artifact in principal exists
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() = 0;

        /// Load as a blob
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBlob(Keep keep, ISlangBlob** outBlob) = 0;
    
        /// Require artifact is available as a file.
        /// NOTE! May need to serialize and write as a temporary file.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFile(Keep keep, IFileArtifactRepresentation** outFileRep) = 0;

        /// Get the name of the artifact. This can be empty.
    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() = 0;

        /// Find an item by casting it's interface
    virtual SLANG_NO_THROW void* SLANG_MCALL findItemInterface(const Guid& uuid) = 0;
        /// Only works on ICastable derived items. Can find interfaces or objects.
    virtual SLANG_NO_THROW void* SLANG_MCALL findItemObject(const Guid& classGuid) = 0;

        /// Add a representation
    virtual SLANG_NO_THROW void SLANG_MCALL addItem(ISlangUnknown* item) = 0;
        /// Get the item at the index
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getItemAt(Index i) = 0;
        /// Remove the element at the specified index. 
    virtual SLANG_NO_THROW void SLANG_MCALL removeItemAt(Index i) = 0;
        /// Get the amount of elements
    virtual SLANG_NO_THROW Index SLANG_MCALL getItemCount() = 0;
};

/* A list of artifacts. */
class IArtifactList : public ICastable
{
    SLANG_COM_INTERFACE(0x5ef6ace5, 0xc928, 0x4c7b, { 0xbc, 0xba, 0x83, 0xa9, 0xd9, 0x66, 0x64, 0x27 })

        /// Get the artifact this list belongs to. Can be nullptr.
        /// Note this is a *weak* reference.
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL getParent() = 0;
        /// The parent is no longer accessible
    virtual SLANG_NO_THROW void SLANG_MCALL setParent(IArtifact* artifact) = 0;

        /// Get the artifact at the specified index
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL getAt(Index index) = 0;
        /// Get the count of all the artifacts
    virtual SLANG_NO_THROW Count SLANG_MCALL getCount() = 0;
        /// Add the artifact to the list
    virtual SLANG_NO_THROW void SLANG_MCALL add(IArtifact* artifact) = 0;
        /// Removes at index, keeps other artifacts in the same order
    virtual SLANG_NO_THROW void SLANG_MCALL removeAt(Index index) = 0;
        /// Clear the list
    virtual SLANG_NO_THROW void SLANG_MCALL clear() = 0;
};

class ArtifactList : public ComBaseObject, public IArtifactList
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // IArtifactList
    SLANG_NO_THROW IArtifact* SLANG_MCALL getParent() SLANG_OVERRIDE { return m_parent; }
    SLANG_NO_THROW void SLANG_MCALL setParent(IArtifact* parent) SLANG_OVERRIDE { _setParent(parent); }

    SLANG_NO_THROW IArtifact* SLANG_MCALL getAt(Index index) SLANG_OVERRIDE { return m_artifacts[index]; }
    SLANG_NO_THROW Count SLANG_MCALL getCount() SLANG_OVERRIDE { return m_artifacts.getCount(); }
    SLANG_NO_THROW void SLANG_MCALL add(IArtifact* artifact) SLANG_OVERRIDE;
    SLANG_NO_THROW void SLANG_MCALL removeAt(Index index) SLANG_OVERRIDE;
    SLANG_NO_THROW void SLANG_MCALL clear() SLANG_OVERRIDE;

        // NOTE! The parent is a weak reference. 
    ArtifactList(IArtifact* parent):
        m_parent(parent)
    {
    }

    virtual ~ArtifactList() { _setParent(nullptr); }

protected:
    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    void _setParent(IArtifact* artifact);
    
    IArtifact* m_parent;
    List<ComPtr<IArtifact>> m_artifacts;
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
class Artifact : public ComBaseObject, public IArtifact
{
public:
    
    SLANG_COM_BASE_IUNKNOWN_ALL
    
        /// IArtifact impl
    virtual SLANG_NO_THROW Desc SLANG_MCALL getDesc() SLANG_OVERRIDE { return m_desc; }
    virtual SLANG_NO_THROW IArtifact* SLANG_MCALL getParent() SLANG_OVERRIDE { return m_parent; }
    virtual SLANG_NO_THROW void SLANG_MCALL setParent(IArtifact* parent) SLANG_OVERRIDE { m_parent = parent; }
    virtual SLANG_NO_THROW bool SLANG_MCALL exists() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBlob(Keep keep, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL requireFile(Keep keep, IFileArtifactRepresentation** outFileRep) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getName() SLANG_OVERRIDE { return m_name.getBuffer(); }
    virtual SLANG_NO_THROW void* SLANG_MCALL findItemInterface(const Guid& uuid) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void* SLANG_MCALL findItemObject(const Guid& classGuid) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL addItem(ISlangUnknown* intf) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getItemAt(Index i) SLANG_OVERRIDE { return m_items[i]; }
    virtual SLANG_NO_THROW void SLANG_MCALL removeItemAt(Index i) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW Index SLANG_MCALL getItemCount() SLANG_OVERRIDE { return m_items.getCount(); }

    /// Ctor
    Artifact(const Desc& desc, const String& name) :
        m_desc(desc),
        m_name(name),
        m_parent(nullptr)
    {}

protected:
    void* getInterface(const Guid& uuid);

    Desc m_desc;                                ///< Description of the artifact
    IArtifact* m_parent;                        ///< Artifact this artifact belongs to

    String m_name;                              ///< Name of this artifact

    List<ComPtr<ISlangUnknown>> m_items;        ///< Associated items
};

// Helper template to make finding an item more simple
// There isn't a problem if we only have a forward declaration, because in that case T::getTypeGuid can't work.
SLANG_FORCE_INLINE void* _findItemImpl(IArtifact* artifact, const Guid& guid, const ISlangUnknown* intf)
{
    SLANG_UNUSED(intf);
    return artifact->findItemInterface(guid);
}

SLANG_FORCE_INLINE void* _findItemImpl(IArtifact* artifact, const Guid& guid, const ICastable* castable)
{
    SLANG_UNUSED(castable);
    return artifact->findItemObject(guid);
}

SLANG_FORCE_INLINE void* _findItemImpl(IArtifact* artifact, const Guid& guid, const void* other)
{
    SLANG_UNUSED(other);
    return artifact->findItemObject(guid);
}

template <typename T>
SLANG_FORCE_INLINE T* findItem(IArtifact* artifact)
{
    return (T*)_findItemImpl(artifact, T::getTypeGuid(), (T*)nullptr);
}

} // namespace Slang

#endif
