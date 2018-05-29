// render.h
#pragma once

#include "window.h"

//#include "shader-input-layout.h"

#include "../../source/core/slang-result.h"
#include "../../source/core/smart-pointer.h"
#include "../../source/core/list.h"

namespace renderer_test {

// Had to move here, because Options needs types defined here
typedef intptr_t Int;
typedef uintptr_t UInt;

// pre declare types
class Surface;

// Declare opaque type
class InputLayout: public Slang::RefObject
{
	public:
};

enum class PipelineType
{
    Unknown,
    Graphics,
    Compute,
    CountOf,
};

enum class RendererType
{
    Unknown,
    DirectX11,
    DirectX12,
    OpenGl,
    Vulkan,
    CountOf,
};

enum class ProjectionStyle
{
    Unknown,
    OpenGl,
    DirectX,
    Vulkan, 
    CountOf,
};

/// The style of the binding
enum class BindingStyle
{
    Unknown,
    DirectX,
    OpenGl,
    Vulkan,
    CountOf,
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

/// Different formats of things like pixels or elements of vertices
/// NOTE! Any change to this type (adding, removing, changing order) - must also be reflected in changes to RendererUtil 
enum class Format
{
    Unknown,

    RGBA_Float32,
    RGB_Float32,
    RG_Float32,
    R_Float32,

    RGBA_Unorm_UInt8,

    D_Float32, 
    D_Unorm24_S8,

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
    Unknown,                    ///< Unknown mapping type
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
        bool canBind(BindFlag::Enum bindFlag) const { return (bindFlags & bindFlag) != 0; }
        bool hasCpuAccessFlag(AccessFlag::Enum accessFlag) { return (cpuAccessFlags & accessFlag) != 0; }

        Type type = Type::Unknown;

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
    bool canBind(BindFlag::Enum bindFlag) const { return getDescBase().canBind(bindFlag); }

        /// For a usage gives the required binding flags
    static const BindFlag::Enum s_requiredBinding[];    /// Maps Usage to bind flags required  

    protected:
    Resource(Type type):
        m_type(type)
    {}

    static void compileTimeAsserts();

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
        void init(Type typeIn);
            /// Initialize different dimensions. For cubemap, use init2D
        void init1D(Format format, int width, int numMipMaps = 0);
        void init2D(Type typeIn, Format format, int width, int height, int numMipMaps = 0);
        void init3D(Format format, int width, int height, int depth, int numMipMaps = 0);

            /// Given the type, calculates the number of mip maps. 0 on error
        int calcNumMipLevels() const;
            /// Calculate the total number of sub resources. 0 on error.
        int calcNumSubResources() const;

            /// Calculate the effective array size - in essence the amount if mip map sets needed. 
            /// In practice takes into account if the arraySize is 0 (it's not an array, but it will still have at least one mip set) 
            /// and if the type is a cubemap (multiplies the amount of mip sets by 6) 
        int calcEffectiveArraySize() const;

            /// Use type to fix the size values (and array size). 
            /// For example a 1d texture, should have height and depth set to 1.
        void fixSize();

            /// Set up default parameters based on type and usage
        void setDefaults(Usage initialUsage);

        Size size; 

        int arraySize;          ///< Array size 

        int numMipLevels;       ///< Number of mip levels - if 0 will create all mip levels
        Format format;          ///< The resources format
        SampleDesc sampleDesc;  ///< How the resource is sampled
    };

        /// The ordering of the subResources is
        /// forall (effectiveArraySize)
        ///     forall (mip levels)
        ///         forall (depth levels)
    struct Data
    {
        ptrdiff_t* mipRowStrides;           ///< The row stride for a mip map
        int numMips;                        ///< The number of mip maps 
        const void*const* subResources;     ///< Pointers to each full mip subResource 
        int numSubResources;                ///< The total amount of subResources. Typically = numMips * depth * arraySize 
    };

        /// Get the description of the texture
    SLANG_FORCE_INLINE const Desc& getDesc() const { return m_desc; }

        /// Ctor
    TextureResource(const Desc& desc):
        Parent(desc.type),
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

    typedef uint16_t BindIndex;

        /// Shader binding style
    enum class ShaderStyle
    {
        Hlsl,
        Glsl,
        CountOf,
    };

    struct ShaderStyleFlag
    {
        enum Enum 
        {
            Hlsl = 1 << int(ShaderStyle::Hlsl),
            Glsl = 1 << int(ShaderStyle::Glsl),
        };
    };
    typedef int ShaderStyleFlags;           ///< Combination of ShaderStyleFlag 

        /// A 'compact' representation of a 0 or more BindIndices.  
        /// A Slice in this context is effectively an unowned array. 
        /// If only a single index is he held (which is common) it's held directly in the m_indexOrBase member, otherwise m_indexOrBase is an index into the 
        /// m_indices list of the Desc. Can be turned into a BindIndexSlice (which is easier to use, and iterable) using asBindIndexSlice method on Desc 
    struct CompactBindIndexSlice
    {
        typedef uint16_t SizeType;
        /// Default Ctor makes an empty set
        SLANG_FORCE_INLINE CompactBindIndexSlice() :
            m_size(0),
            m_indexOrBase(0)
        {}
            /// Ctor for one or more. NOTE! Meaning if indexIn changes depending if numIndices > 1.
        SLANG_FORCE_INLINE CompactBindIndexSlice(int indexIn, int sizeIn) :
            m_size(SizeType(sizeIn)),
            m_indexOrBase(BindIndex(indexIn))
        {
        }
        SizeType m_size;
        BindIndex m_indexOrBase;                 ///< Meaning changes depending on numIndices. If 1, it is the index if larger than 1, then is an index into 'indices'  
    };

        /// Holds the BindIndex slice associated with each ShaderStyle
    struct ShaderBindSet
    {
        void set(ShaderStyle style, const CompactBindIndexSlice& slice) { shaderSlices[int(style)] = slice; }
        void setAll(const CompactBindIndexSlice& slice) 
        {
            for (int i = 0; i < int(ShaderStyle::CountOf); ++i)
            {
                shaderSlices[i] = slice;
            }
        }

        CompactBindIndexSlice shaderSlices[int(ShaderStyle::CountOf)];
    };

        /// A slice (non owned array) of BindIndices
        /// TODO: have a generic Slice<T> type instead of this specific type
    struct BindIndexSlice
    {
        const BindIndex* begin() const { return data; }
        const BindIndex* end() const { return data + size; }

        int indexOf(BindIndex index) const 
        {
            for (int i = 0; i < size; ++i)
            {
                if (data[i] == index) 
                {
                    return i;
                }   
            }
            return -1;
        }

        int getSize() const { return int(size); }
        BindIndex operator[](int i) const { assert(i >= 0 && i < size); return data[i]; }

        const BindIndex* data;
        int size;
    };

    struct SamplerDesc
    {
        bool isCompareSampler;
    };

    struct Binding
    {
        BindingType bindingType;                ///< Type of binding
        int descIndex;                          ///< The description index associated with type. -1 if not used. For example if bindingType is Sampler, the descIndex is into m_samplerDescs.
        Slang::RefPtr<Resource> resource;       ///< Associated resource. nullptr if not used
        ShaderBindSet shaderBindSet;            ///< Holds BindIndices associated with each ShaderStyle 
    };

    struct Desc
    {        
            /// Given a RegisterSet, return as a RegisterList, that can be easily iterated over
        BindIndexSlice asSlice(const CompactBindIndexSlice& set) const;
            /// Given a RegisterDesc and a style returns a RegisterList, that can be easily iterated over
        BindIndexSlice asSlice(ShaderStyle style, const ShaderBindSet& shaderBindSet) const;

            /// Returns the first member of the set, or returns -1 if is empty
        int getFirst(const CompactBindIndexSlice& set) const;
            /// Returns the first member of the set, or returns -1 if is empty
        int getFirst(ShaderStyle style, const ShaderBindSet& shaderBindSet) const;

            /// Add a resource - assumed that the binding will match the Desc of the resource
        void addResource(BindingType bindingType, Resource* resource, const ShaderBindSet& shaderBindSet);
            /// Add a sampler        
        void addSampler(const SamplerDesc& desc, const ShaderBindSet& shaderBindSet);
            /// Add a BufferResource 
        void addBufferResource(BufferResource* resource, const ShaderBindSet& shaderBindSet) { addResource(BindingType::Buffer, resource, shaderBindSet); }
            /// Add a texture 
        void addTextureResource(TextureResource* resource, const ShaderBindSet& shaderBindSet) { addResource(BindingType::Texture, resource, shaderBindSet); }
            /// Add combined texture a
        void addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const ShaderBindSet& shaderBindSet);

            /// Clear the contents 
        void clear();

            /// Given an index, makes a CompactBindIndexSlice. If index is < 0, assumes means no indices, and just returns the empty slice
        CompactBindIndexSlice makeCompactSlice(int index);
            /// Given a list of indices, makes a CompactBindIndexSlice. Note does check for indices being unique and the order is maintained. 
            /// Only >= 0 indices are valid
        CompactBindIndexSlice makeCompactSlice(const int* indices, int numIndices);

            /// Returns the index of the element in the slice
        int indexOf(const CompactBindIndexSlice& slice, BindIndex index) const { return asSlice(slice).indexOf(index); }

            /// Find the 
        int findBindingIndex(Resource::BindFlag::Enum bindFlag, ShaderStyleFlags shaderStyleFlags, BindIndex index) const;

        Slang::List<Binding> m_bindings;                            ///< All of the bindings in order
        Slang::List<SamplerDesc> m_samplerDescs;                    ///< Holds the SamplerDesc for the binding - indexed by the descIndex member of Binding 
        Slang::List<BindIndex> m_sharedBindIndices;                 ///< Used to store BindIndex slices that don't fit into CompactBindIndexSlice
        int m_numRenderTargets = 1;
    };

        /// Get the Desc used to create this binding
    SLANG_FORCE_INLINE const Desc& getDesc() const { return m_desc; }

    protected:
    BindingState(const Desc& desc):
        m_desc(desc)
    {
    }

    Desc m_desc;
};

class Renderer: public Slang::RefObject
{
public:
    
    struct Desc
    {
        int width;          ///< Width in pixels
        int height;         ///< height in pixels
    };

    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) = 0;

    virtual void setClearColor(const float color[4]) = 0;
    virtual void clearFrame() = 0;

    virtual void presentFrame() = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed. 
    virtual TextureResource* createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData = nullptr) { return nullptr; }
        /// Create a buffer resource
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& desc, const void* initData = nullptr) { return nullptr; } 

        /// Captures the back buffer and stores the result in surfaceOut. If the surface contains data - it will either be overwritten (if same size and format), or freed and a re-allocated.
    virtual SlangResult captureScreenSurface(Surface& surfaceOut) = 0;

    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) = 0;
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

    virtual void draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual void dispatchCompute(int x, int y, int z) = 0;

        /// Commit any buffered state changes or draw calls. 
        /// presentFrame will commitAll implicitly before doing a present
    virtual void submitGpuWork() = 0;
        /// Blocks until Gpu work is complete
    virtual void waitForGpu() = 0;

        /// Get the type of this renderer
    virtual RendererType getRendererType() const = 0;
};

// ----------------------------------------------------------------------------------------
inline void Renderer::setVertexBuffer(UInt slot, BufferResource* buffer, UInt stride, UInt offset)
{
    setVertexBuffers(slot, 1, &buffer, &stride, &offset);
}

/// Functions that are around Renderer and it's types
struct RendererUtil
{
        /// Gets the size in bytes of a Format type. Returns 0 if a size is not defined/invalid
    SLANG_FORCE_INLINE static size_t getFormatSize(Format format) { return s_formatSize[int(format)]; }
        /// Given a renderer type, gets a projection style
    static ProjectionStyle getProjectionStyle(RendererType type);

        /// Given the projection style returns an 'identity' matrix, which ensures x,y mapping to pixels is the same on all targets
    static void getIdentityProjection(ProjectionStyle style, float projMatrix[16]);

        /// Get the binding style from the type
    static BindingStyle getBindingStyle(RendererType type) { return s_rendererTypeToBindingStyle[int(type)]; }

    private:
    static void compileTimeAsserts();
    static const uint8_t s_formatSize[]; // Maps Format::XXX to a size in bytes;
    static const BindingStyle s_rendererTypeToBindingStyle[];           ///< Maps a RendererType to a BindingStyle
};

} // renderer_test
