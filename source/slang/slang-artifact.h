// slang-artifact.h
#ifndef SLANG_ARTIFACT_H
#define SLANG_ARTIFACT_H

#include "slang-compiler.h" 

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


/**
A value type to describe aspects of a CompileArtifact.
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
    bool isGpuBinary() const { return isPayloadCpuBinary(payload); }

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

        /// Given a code gen target, get the equivalent ArtifactDesc
    static This make(CodeGenTarget target);

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

/* The Artifact can contain multiple entries which represent the contained data in different forms. */
class Artifact : public RefObject
{
public:

    typedef ArtifactDesc Desc;

    typedef ArtifactKind Kind;
    typedef ArtifactPayload Payload;
    typedef ArtifactStyle Style;
    typedef ArtifactFlags Flags;

    // Controls what items can be kept. 
    enum class Keep
    {
        No,         ///< Don't keep the item
        Yes,        ///< Yes keep the final item
        All,        ///< Keep the final item and any intermediataries
    };

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
        // TODO(JS): We may want distinguish between entries which are representations of the
        // archive itself or
        //
        // * Contained items (for example the paths of contained Artifacts)
        // * Ancilliary items (for example a custom ISlangFileSystem could be associated with an Artifact)
        
        /// NOTE! Only interface innstances work across dll/shared library boundaries
        /// because casting other types does not work across those boundaries.

        // The Type of the entry
        enum class Type
        {
            InterfaceInstance,          ///< An interface instance 
            ObjectInstance,             ///< An object instance
            RawInstance,
        };

        Type type;
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

    void add(RefObject* obj);
    void add(ISlangUnknown* intf);

    PathType getPathType() const { return m_pathType;  }
    const String& getPath() const { return m_path;  }

    const List<Entry>& getEntries() const { return m_entries; }

        /// True if can keep an intermediate item
    static bool canKeepIntermediate(Keep keep) { return keep == Keep::All; }
        /// True if can keep
    static bool canKeep(Keep keep) { return Index(keep) >= Index(Keep::Yes); }

        /// Returns the keep type for an intermediate
    static Keep getIntermediateKeep(Keep keep) { return (keep == Keep::All) ? Keep::All : Keep::No;  }

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


// Class to hold information serialized in from a -r slang-lib/slang-module
class ModuleLibrary : public RefObject
{
public:

    List<FrontEndCompileRequest::ExtraEntryPointInfo> m_entryPoints;
    List<RefPtr<IRModule>> m_modules;
};

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

// Given a product make available as a module
SlangResult loadModuleLibrary(Artifact::Keep keep, Artifact* artifact, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

} // namespace Slang

#endif
