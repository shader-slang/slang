// slang-artifact.h
#ifndef SLANG_ARTIFACT_H
#define SLANG_ARTIFACT_H

#include "../core/slang-basic.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

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
    Callable,                   ///< Callable directly (typically means there isn't a binary artifact)

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

// Controls what items can be kept. 
enum class ArtifactKeep
{
    No,         ///< Don't keep the item
    Yes,        ///< Yes keep the final item
    All,        ///< Keep the final item and any intermediataries
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

        /// True if the container appears to be binary linkable
    bool isBinaryLinkable() const;
        /// True if is a CPU binary
    bool isCpuBinary() const { return isPayloadCpuBinary(payload); }
        /// True if is a GPU binary
    bool isGpuBinary() const { return isPayloadGpuBinary(payload); }

        /// Gets the default file extension for the artifact type. Returns empty slice if not known
    UnownedStringSlice getDefaultExtension();

    static UnownedStringSlice getDefaultExtensionForPayload(Payload payload);

        /// Get the extension for CPU/Host for a kind
    static UnownedStringSlice getCpuExtensionForKind(Kind kind);

        /// Returns true if the kind is binary linkable 
    static bool isKindBinaryLinkable(Kind kind);

        /// Returns true if the payload type is CPU
    static bool isPayloadCpuBinary(Payload payload);
        /// Returns true if the payload type is applicable to the GPU
    static bool isPayloadGpuBinary(Payload payload);

        /// True if the payload type is in principal binary linkable
    static bool isPayloadGpuBinaryLinkable(Payload payload);

        /// Try to determine the desc from a path
    static This fromPath(const UnownedStringSlice& slice);
        /// Try to determine the desc from just a file extension (passed without .)
    static This fromExtension(const UnownedStringSlice& slice);

    bool operator==(const This& rhs) const { return kind == rhs.kind && payload == rhs.payload && style == rhs.style && flags == rhs.flags;  }
    bool operator!=(const This& rhs) const { return !(*this == rhs); }

        /// Given a code gen target, get the equivalent ArtifactDesc
    static This makeFromCompileTarget(SlangCompileTarget target);

        /// Construct from the elements
    static This make(Kind inKind, Payload inPayload, Style inStyle = Style::Kernel, Flags flags = 0)
    {
        return This{ inKind, inPayload, inStyle, flags };
    }
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

/* The Artifact type is a type designed to represent some Artifact of compilation. It could be input to or output from a compilation.

An abstraction is desirable here, because depending on the compiler the artifact/s could be

* A file on the file system
* A blob
* Multiple files
* Some other (perhaps multiple) in memory representations 

The artifact uses the Blob as the standard representation of in memory data. 

Some downstream compilers require the artifact to be available as a file system file, or to produce
artifacts that are files. The Artifact type allows to abstract away this difference, including the
ability to turn an in memory representation into a temporary file on the system file. 

The mechanism also allows for 'Containers' which allow for Artifacts to contain other Artifacts (amongst other things).
Those artifacts may be other files. For example a downstream compilation that produces results as well as temporary
files could be a Container containing artifacts for

* Diagnostics
* Temporary files (of known and unknown types)
* Files that contain known types
* Callable interface (an ISlangSharedLibrary)

A more long term goal would be to

* Make Artifact an interface (such that it can work long term over binary boundaries)
* Make Diagnostics into an interface (such it can be added to a Artifact result)
* Use Artifact and related types for downstream compiler 
*/
class Artifact : public RefObject
{
public:

    typedef ArtifactDesc Desc;

    typedef ArtifactKind Kind;
    typedef ArtifactPayload Payload;
    typedef ArtifactStyle Style;
    typedef ArtifactFlags Flags;
    typedef ArtifactKeep Keep;
    
    enum PathType
    {
        None,
        Temporary,
        Existing,
    };

    /* A compile product can be made up of multiple representations.
    */
    struct Entry
    {
        /// NOTE! Only interface innstances work across dll/shared library boundaries
        /// because casting other types does not work across those boundaries.

        // The Type of the entry
        enum class Type : uint8_t
        {
            InterfaceInstance,          ///< An interface instance 
            ObjectInstance,             ///< An object instance
            RawInstance,
        };
        enum class Style : uint8_t
        {
            Artifact,       ///< Means this entry *can* represent the whole artifact
            Child,          ///< Some part of the artifact
            Info,           ///< Informational
            Other,          ///< Other
        };

        Type type;
        Style style;
        union
        {
            RefObject* object;
            ISlangUnknown* intf;
            void* raw;
        };
    };

        /// Given a type T find the associated instance
    template <typename T>
    T* findObjectInstance();

        /// Finds an instance of that has the guid.
    ISlangUnknown* findInterfaceInstance(const Guid& guid);

        /// Returns true if the artifact in principal exists (it could be invalid)
    bool exists() const;

        /// Load as a blob
    SlangResult loadBlob(Keep keep, ComPtr<ISlangBlob>& outBlob);
    
        /// Get as a file. May need to serialize and write as a temporary file.
    SlangResult requireFilePath(Keep keep, String& outPath);

    SLANG_FORCE_INLINE const Desc& getDesc() { return m_desc; }

        /// Returns the index of the entry
    Index indexOf(Entry::Type type) const;
    
        /// Add items
    void setPath(PathType pathType, const String& filePath) { m_pathType = pathType; m_path = filePath; }
    void setBlob(ISlangBlob* blob) { m_blob = blob; }

    void add(Entry::Style style, RefObject* obj);
    void add(Entry::Style style, ISlangUnknown* intf);

    PathType getPathType() const { return m_pathType;  }
    const String& getPath() const { return m_path;  }

    const List<Entry>& getEntries() const { return m_entries; }

        /// Ctor
    Artifact(const Desc& desc) :m_desc(desc) {}
        /// Dtor
    ~Artifact();

protected:
    Desc m_desc;

    PathType m_pathType = PathType::None;       ///< What the path indicates
    String m_path;                              ///< The path 

    ComPtr<ISlangBlob> m_blob;                  ///< Blob to store result in memory

    List<Entry> m_entries;
};

// ----------------------------------------------------------------------
template <typename T>
T* Artifact::findObjectInstance()
{
    RefObject* check = static_cast<T*>(nullptr);
    SLANG_UNUSED(check);

    // Check if we already have it
    for (const auto& entry : m_entries)
    {
        if (entry.type == Entry::Type::ObjectInstance)
        {
            auto obj = as<T>(entry.object);
            if (obj)
            {
                return obj;
            }
        }
    }
    return nullptr;
}

/// True if can keep an intermediate item
SLANG_INLINE  bool canKeepIntermediate(ArtifactKeep keep) { return keep == ArtifactKeep::All; }
    /// True if can keep
SLANG_INLINE bool canKeep(ArtifactKeep keep) { return Index(keep) >= Index(ArtifactKeep::Yes); }
    /// Returns the keep type for an intermediate
SLANG_INLINE ArtifactKeep getIntermediateKeep(ArtifactKeep keep) { return (keep == ArtifactKeep::All) ? ArtifactKeep::All : ArtifactKeep::No; }

} // namespace Slang

#endif
