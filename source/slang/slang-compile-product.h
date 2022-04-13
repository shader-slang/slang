// slang-compile-product.h
#ifndef SLANG_COMPILE_PRODUCT_H
#define SLANG_COMPILE_PRODUCT_H

#include "slang-compiler.h" 

namespace Slang
{

/**
A value type to describe aspects of a CompileArtifact.
**/
struct CompileProductDesc
{
public:
    typedef CompileProductDesc This;

    typedef uint32_t PackedBacking;
    enum class Packed : PackedBacking;
    typedef uint8_t FlagsType;

    enum class ContainerType
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
    enum class PayloadType
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

        CountOf,
    };

    enum class Style : uint8_t
    {
        Unknown,                ///< Unknown

        Kernel,                 ///< Compiled as `GPU kernel` style.        
        Host,                   ///< Compiled in `host` style

        CountOf,
    };

        /// Get in packed format
    inline Packed getPacked() const;

        /// True if the container appears to be binary linkable
    bool isBinaryLinkable() const { return isBinaryLinkable(containerType); }
        /// True if is a CPU payload
    bool isCpu() const { return isCpu(payloadType); }

        /// Gets the default file extension for the artifact type. Returns empty slice if not known
    UnownedStringSlice getDefaultExtension();

    static UnownedStringSlice getDefaultExtensionForPayloadType(PayloadType payloadType);

        /// Returns true if the container type is binary linkable 
    static bool isBinaryLinkable(ContainerType containerType);
        /// Returns true if the payload type is CPU
    static bool isCpu(PayloadType payloadType);

        /// Given a code gen target, get the equivalent CompileProductDesc
    static This make(CodeGenTarget target);

        /// Construct from the elements
    static This make(ContainerType inContainerType, PayloadType inPayloadType, Style inStyle = Style::Kernel, FlagsType flags = 0)
    {
        return This{ inContainerType, inPayloadType, inStyle, flags };
    }
        /// Construct from the packed format
    inline static This make(Packed inPacked);

    ContainerType containerType;
    PayloadType payloadType;
    Style style;
    FlagsType flags;
};

// --------------------------------------------------------------------------
inline CompileProductDesc::Packed CompileProductDesc::getPacked() const
{
    typedef PackedBacking IntType;
    return Packed((IntType(containerType) << 24) |
        (IntType(payloadType) << 16) |
        (IntType(style) << 8) |
        flags);
}

// --------------------------------------------------------------------------
inline /* static */CompileProductDesc CompileProductDesc::make(Packed inPacked)
{
    const PackedBacking packed = PackedBacking(inPacked);

    This r;
    r.containerType = ContainerType(packed >> 24);
    r.payloadType = PayloadType(uint8_t(packed >> 16));
    r.style = Style(uint8_t(packed >> 8));
    r.flags = uint8_t(packed);

    return r;
}

/* The CompileProduct can contain multiple entries which represent the contained data in different forms. */
class CompileProduct : public RefObject
{
public:

    enum class Cache
    {
        Yes,
        No,
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
        /// NOTE! Instance types in general won't work across dll/shared library boundaries
        /// because casting does not work across those boundaries.
        
        // The Type of the entry
        enum class Type
        {
            InterfaceInstance,          ///< An interface instance 
            ObjectInstance,             ///< An object instance
            RawPtr,
        };

        Type type;
        union
        {
            RefObject* object;
            ISlangUnknown* intf;
            void* rawPointer;
        };
    };

        /// Given a type T find the associated instance
    template <typename T>
    T* findObjectInstance();

        /// Load as a blob
    SlangResult loadBlob(Cache cacheBehavior, ComPtr<ISlangBlob>& outBlob);
    
        /// Get as a file. May need to serialize and write as a temporary file.
    SlangResult requireFilePath(String& outPath);

    SLANG_FORCE_INLINE const CompileProductDesc& getDesc() { return m_desc; }

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

        /// Ctor
    CompileProduct(const CompileProductDesc& desc) :m_desc(desc) {}
        /// Dtor
    ~CompileProduct();

protected:
    CompileProductDesc m_desc;

    PathType m_pathType = PathType::None;       ///< What the path indicates
    String m_path;                              ///< The path 

    ComPtr<ISlangBlob> m_blob;                  ///< Blob to store result in memory

    List<Entry> m_entries;
};

// Class to hold information serialized in from a -r slang-lib/slang-module
class ModuleLibrary : public RefObject
{
public:

    List<FrontEndCompileRequest::ExtraEntryPointInfo> m_entryPoints;
    List<RefPtr<IRModule>> m_modules;
};

// ----------------------------------------------------------------------
template <typename T>
T* CompileProduct::findObjectInstance()
{
    RefObject* check = static_cast<T*>(nullptr);
    SLANG_UNUSED(check);

    // Check if we already have it
    for (const auto& entry : m_entries)
    {
        if (entry.type == CompileProduct::Entry::Type::ObjectInstance)
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

SlangResult loadModuleLibrary(const Byte* inBytes, size_t bytesCount, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

// Given a product make available as a module
SlangResult loadModuleLibrary(CompileProduct::Cache cacheBehavior, CompileProduct* product, EndToEndCompileRequest* req, RefPtr<ModuleLibrary>& module);

} // namespace Slang

#endif
