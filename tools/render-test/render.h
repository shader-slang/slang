// render.h
#pragma once

#include "options.h"
#include "window.h"
#include "shader-input-layout.h"

#include "../../source/core/slang-result.h"
#include "../../source/core/smart-pointer.h"
#include "../../source/core/list.h"

namespace renderer_test {


// Declare opaque type
class InputLayout: public Slang::RefObject
{
	public:
};

class ShaderProgram: public Slang::RefObject
{
	public:
};



struct ShaderCompileRequest
{
    struct SourceInfo
    {
        char const* path;

        // The data may either be source text (in which
        // case it can be assumed to be nul-terminated with
        // `dataEnd` pointing at the terminator), or
        // raw binary data (in which case `dataEnd` points
        // at the end of the buffer).
        char const* dataBegin;
        char const* dataEnd;
    };

    struct EntryPoint
    {
        char const* name = nullptr;
        char const* profile = nullptr;

        SourceInfo  source;
    };

    SourceInfo source;
    EntryPoint vertexShader;
    EntryPoint fragmentShader;
    EntryPoint computeShader;
    Slang::List<Slang::String> entryPointTypeArguments;
};

class ShaderCompiler
{
public:
    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) = 0;
};

enum class Format
{
    Unknown,
    RGB_Float32,
    RG_Float32,
    R_Float32,

    RGBA_Unorm_UInt8,
    CountOf, 
};

struct InputElementDesc
{
    char const* semanticName;
    UInt        semanticIndex;
    Format      format;
    UInt        offset;
};

enum class MapFlavor
{
    HostRead,
    HostWrite,
    WriteDiscard,
};

enum class PrimitiveTopology
{
    TriangleList,
};

class Resource: public Slang::RefObject
{
    public:

        /// The type of resource.
        /// NOTE! The order needs to be such that all texture types are at or after Texture1D (otherwise isTexture won't work correctly)
    enum class Type
    {
        Unknown,            ///< Unknown
        Buffer,             ///< A buffer (like a constant/index/vertex buffer)
        Texture1D,          ///< A 1d texture
        Texture2D,          ///< A 2d texture
        Texture3D,          ///< A 3d texture
        TextureCube,        ///< A cubemap consists of 6 Texture2D like faces
        CountOf,
    };

        /// Describes how a resource is to be used
    enum class Usage
    {
        Unknown = -1,
        VertexBuffer = 0,
        IndexBuffer,
        ConstantBuffer,
        StreamOutput,
        RenderTarget,
        DepthRead,
        DepthWrite,
        UnorderedAccess,
        PixelShaderResource,
        NonPixelShaderResource,
        GenericRead,
        CountOf,
    };

        /// Binding flags describe all of the ways a resource can be bound - and therefore used
    struct BindFlag
    {
        enum Enum
        {
            VertexBuffer            = 0x001,
            IndexBuffer             = 0x002, 
            ConstantBuffer          = 0x004, 
            StreamOutput            = 0x008, 
            RenderTarget            = 0x010, 
            DepthStencil            = 0x020, 
            UnorderedAccess         = 0x040, 
            PixelShaderResource     = 0x080, 
            NonPixelShaderResource  = 0x100, 
        };
    };

        /// Combinations describe how a resource can be accessed (typically by the host/cpu)
    struct AccessFlag
    {
        enum Enum
        {
            Read = 0x1,
            Write = 0x2
        };
    };

        /// Base class for Descs
    struct DescBase
    {
        int bindFlags = 0;          ///< Combination of Resource::BindFlag or 0 (and will use initialUsage to set)
        int cpuAccessFlags = 0;     ///< Combination of Resource::AccessFlag 
    };

        /// Get the type
    SLANG_FORCE_INLINE Type getType() const { return m_type; }
        /// True if it's a texture derived type
    SLANG_FORCE_INLINE bool isTexture() const { return int(m_type) >= int(Type::Texture1D); }
        /// True if it's a buffer derived type
    SLANG_FORCE_INLINE bool isBuffer() const { return m_type == Type::Buffer; }

        /// Get the descBase
    const DescBase& getDescBase() const;
        /// Returns true if can bind with flag
    bool canBind(BindFlag::Enum bindFlag) const { return (getDescBase().bindFlags & bindFlag) != 0; }

        /// For a usage gives the required binding flags
    static const BindFlag::Enum s_requiredBinding[int(Usage::CountOf)]; 

    protected:
    Resource(Type type):
        m_type(type)
    {}

    Type m_type;
};

class BufferResource: public Resource
{
    public:
    typedef Resource Parent;

    struct Desc: public DescBase
    {
        void init(size_t sizeInBytesIn)
        {
            sizeInBytes = sizeInBytesIn;
            elementSize = 0;
        }
            /// Set up default parameters based on usage
        void setDefaults(Usage initialUsage);

        size_t sizeInBytes;     ///< Total size in bytes 
        int elementSize;        ///< Get the element stride. If > 0, this is a structured buffer
    };

        /// Get the buffer description
    SLANG_FORCE_INLINE const Desc& getDesc() const { return m_desc; }

        /// Ctor
    BufferResource(const Desc& desc):
        Parent(Type::Buffer),
        m_desc(desc)
    {
    }

    protected:
    Desc m_desc;
};

class TextureResource: public Resource
{
    public:
    typedef Resource Parent;

    struct SampleDesc
    {
        void init()
        {
            numSamples = 1;
            quality = 0;
        }
        int numSamples;                     ///< Number of samples per pixel
        int quality;                        ///< The quality measure for the samples
    };
    
    struct Size
    {
        void init()
        {
            width = height = depth = 1;
        }
        void init(int widthIn, int heightIn = 1, int depthIn = 1)
        {
            width = widthIn;
            height = heightIn; 
            depth = depthIn; 
        }
            /// Given the type works out the maximum dimension size
        int calcMaxDimension(Type type) const;
            /// Given a size, calculates the size at a mip level
        Size calcMipSize(int mipLevel) const;

        int width;              ///< Width in pixels
        int height;             ///< Height in pixels (if 2d or 3d)
        int depth;              ///< Depth (if 3d) 
    };

    struct Desc: public DescBase
    {
            /// Initialize with default values
        void init();
            /// Initialize different dimensions. For cubemap, use init2D
        void init1D(Format format, int width, int numMipMaps = 0);
        void init2D(Format format, int width, int height, int numMipMaps = 0);
        void init3D(Format format, int width, int height, int depth, int numMipMaps = 0);

            /// Given the type, calculates the number of mip maps. 0 on error
        int calcNumMipLevels(Type type) const;
            /// Calculate the total number of sub resources. 0 on error.
        int calcNumSubResources(Type type) const;

            /// Calculate the effective array size - in essence the amount if mip map sets needed. 
            /// In practice takes into account if the arraySize is 0 (it's not an array, but it will still have at least one mip set) 
            /// and if the type is a cubemap (multiplies the amount of mip sets by 6) 
        int calcEffectiveArraySize(Type type) const;

            /// 
        void fixSize(Type type);

            /// Set up default parameters based on type and usage
        void setDefaults(Type type, Usage initialUsage);

        Size size; 

        int arraySize;          ///< Array size 

        int numMipLevels;       ///< Number of mip levels - if 0 will generate all mip levels
        Format format;          ///< The resources format
        SampleDesc sampleDesc;  ///< How the resource is sampled
    };

        /// The ordering of the subResources is 
        /// forall (cube faces (6) * arraySize / arraySize)
        ///     forall (mip levels)
        ///         forall (depth levels)
    struct Data
    {
        ptrdiff_t* mipRowStrides;        /// The row stride for a mip map
        int numMips;                    ///< The number of mip maps 
        const void*const* subResources;  ///< Pointers to each full mip subResource 
        int numSubResources;            /// The total amount of subResources. Typically = numMips * depth * arraySize 
    };

        /// Get the description of the texture
    SLANG_FORCE_INLINE const Desc& getDesc() const { return m_desc; }

        /// Ctor
    TextureResource(Type type, const Desc& desc):
        Parent(type),
        m_desc(desc)
    {
    }

    SLANG_FORCE_INLINE static int calcMipSize(int width, int mipLevel)
    {
        width = width >> mipLevel;
        return width > 0 ? width : 1;
    }
    
    protected:
    Desc m_desc;
};

enum class BindingType
{
    Unknown,
    Sampler,
    Buffer,
    Texture,
    CombinedTextureSampler,
    CountOf,
};

class BindingState : public Slang::RefObject
{
public:

    enum class ShaderStyle
    {
        Hlsl,
        Glsl,
        CountOf,
    };

    struct RegisterSet
    {
        /// Default Ctor makes an empty set
        SLANG_FORCE_INLINE RegisterSet() :
            m_numIndices(0),
            m_indexOrBase(0)
        {}
        /// Ctor for one or more. NOTE! Meaning if indexIn changes depending if numIndices > 1.
        SLANG_FORCE_INLINE RegisterSet(int indexIn, int numIndicesIn) :
            m_numIndices(uint8_t(numIndicesIn)),
            m_indexOrBase(uint16_t(indexIn))
        {
        }
        uint8_t m_numIndices;
        uint16_t m_indexOrBase;                 ///< Meaning changes depending on numIndices. If 1, it is the index if larger than 1, then is an index into 'indices'  
    };

    struct RegisterDesc
    {
        RegisterSet registerSets[int(ShaderStyle::CountOf)];
    };

    struct RegisterList
    {
        const uint16_t* begin() const { return indices; }
        const uint16_t* end() const { return indices + numIndices; }

        int getSize() const { return int(numIndices); }
        uint16_t operator[](int i) const { assert(i >= 0 && i < numIndices); return indices[i]; }

        const uint16_t* indices;
        size_t numIndices;
    };

    struct SamplerDesc
    {
        bool isCompareSampler;
    };

    struct Desc
    {
        struct Binding
        {
            BindingType bindingType;                ///< Type of binding
            int descIndex;                          ///< The description index associated with type. -1 if not used. For example if bindingType is Sampler, the descIndex is into m_samplerDescs.
            Slang::RefPtr<Resource> resource;       ///< Associated resource. nullptr if not used
            RegisterDesc registerDesc;              ///< Registers associated with binding
        };

            /// Given a RegisterSet, return as a RegisterList, that can be easily iterated over
        RegisterList asRegisterList(const RegisterSet& set) const;
            /// Given a RegisterDesc and a style returns a RegisterList, that can be easily iterated over
        RegisterList asRegisterList(ShaderStyle style, const RegisterDesc& registerDesc) const;

            /// Returns the first member of the set, or returns -1 if is empty
        int getFirst(const RegisterSet& set) const;
            /// Returns the first member of the set, or returns -1 if is empty
        int getFirst(ShaderStyle style, const RegisterDesc& registerDesc) const;

            /// Add a resource - assumed that the binding will match the Desc of the resource
        void addResource(BindingType bindingType, Resource* resource, const RegisterDesc& registerDesc);
            /// Add a sampler        
        void addSampler(const SamplerDesc& desc, const RegisterDesc& registerDesc);
            /// Add a BufferResource 
        void addBufferResource(BufferResource* resource, const RegisterDesc& registerDesc) { addResource(BindingType::Buffer, resource, registerDesc); }
            /// Add a texture 
        void addTextureResource(TextureResource* resource, const RegisterDesc& registerDesc) { addResource(BindingType::Texture, resource, registerDesc); }
            /// Add combined texture a
        void addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const RegisterDesc& registerDesc);

            /// Clear the contents 
        void clear();

            /// Given an index, returns as a register set. If index is < 0, assumes means no indices, and just returns the empty set
        RegisterSet addRegisterSet(int index);
            /// Given a list of indices, returns the associated register set. Note does check for indices being unique. The order is maintained. 
            /// Only >= 0 indices are valid
        RegisterSet addRegisterSet(const int* indices, int numIndices);

        Slang::List<Binding> m_bindings;
        Slang::List<SamplerDesc> m_samplerDescs;
        Slang::List<uint16_t> m_indices;                  ///< Used to store lists of registers
        int m_numRenderTargets = 1;
    };
};

class Renderer: public Slang::RefObject
{
public:
    virtual SlangResult initialize(void* inWindowHandle) = 0;

    virtual void setClearColor(const float color[4]) = 0;
    virtual void clearFrame() = 0;

    virtual void presentFrame() = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed. 
    virtual TextureResource* createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData = nullptr) { return nullptr; }
        /// Create a buffer resource
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& desc, const void* initData = nullptr) { return nullptr; } 

    virtual SlangResult captureScreenShot(const char* outputPath) = 0;
    virtual void serializeOutput(BindingState* state, const char* outputPath) = 0;

    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) = 0;
    virtual BindingState* createBindingState(const ShaderInputLayout& shaderInput) = 0;
    virtual BindingState* createBindingState(const BindingState::Desc& desc) { return nullptr; }
    virtual ShaderCompiler* getShaderCompiler() = 0;

    virtual void* map(BufferResource* buffer, MapFlavor flavor) = 0;
    virtual void unmap(BufferResource* buffer) = 0;

    virtual void setInputLayout(InputLayout* inputLayout) = 0;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual void setBindingState(BindingState* state) = 0;
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) = 0;

    inline void setVertexBuffer(UInt slot, BufferResource* buffer, UInt stride, UInt offset = 0);

    virtual void setShaderProgram(ShaderProgram* program) = 0;

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* offsets) = 0;
    inline void setConstantBuffer(UInt slot, BufferResource* buffer, UInt offset = 0);

    virtual void draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual void dispatchCompute(int x, int y, int z) = 0;

        /// Commit any buffered state changes or draw calls. 
        /// presentFrame will commitAll implicitly before doing a present
    virtual void submitGpuWork() = 0;
        /// Blocks until Gpu work is complete
    virtual void waitForGpu() = 0;
};

// ----------------------------------------------------------------------------------------
inline void Renderer::setVertexBuffer(UInt slot, BufferResource* buffer, UInt stride, UInt offset)
{
    setVertexBuffers(slot, 1, &buffer, &stride, &offset);
}
// ----------------------------------------------------------------------------------------
inline void Renderer::setConstantBuffer(UInt slot, BufferResource* buffer, UInt offset)
{
    setConstantBuffers(slot, 1, &buffer, &offset);
}


} // renderer_test
