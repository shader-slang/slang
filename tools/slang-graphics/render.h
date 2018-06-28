// render.h
#pragma once

#include "window.h"

//#include "shader-input-layout.h"

#include "../../slang-com-helper.h"

#include "../../source/core/smart-pointer.h"
#include "../../source/core/list.h"

namespace slang_graphics {

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

enum class StageType
{
    Unknown,
    Vertex,
    Hull,
    Domain,
    Geometry,
    Fragment,
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

    struct KernelDesc
    {
        StageType   stage;
        void const* codeBegin;
        void const* codeEnd;

        UInt getCodeSize() const { return (char const*)codeEnd - (char const*)codeBegin; }
    };

    struct Desc
    {
        PipelineType        pipelineType;
        KernelDesc const*   kernels;
        Int                 kernelCount;

            /// Find and return the kernel for `stage`, if present.
        KernelDesc const* findKernel(StageType stage) const
        {
            for(Int ii = 0; ii < kernelCount; ++ii)
                if(kernels[ii].stage == stage)
                    return &kernels[ii];
            return nullptr;
        }
    };
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
        SourceInfo  source;
    };

    SourceInfo source;
    EntryPoint vertexShader;
    EntryPoint fragmentShader;
    EntryPoint computeShader;
    Slang::List<Slang::String> entryPointTypeArguments;
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

    R_UInt32,

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
            format = Format::Unknown;
        }
            /// Set up default parameters based on usage
        void setDefaults(Usage initialUsage);

        size_t sizeInBytes;     ///< Total size in bytes
        int elementSize;        ///< Get the element stride. If > 0, this is a structured buffer
        Format format;
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
        /// A register set consists of one or more contiguous indices.
        /// To be valid index >= 0 and size >= 1
    struct RegisterRange
    {
            /// True if contains valid contents
        bool isValid() const { return size > 0; }
            /// True if valid single value
        bool isSingle() const { return size == 1; }
            /// Get as a single index (must be at least one index)
        int getSingleIndex() const { return (size == 1) ? index : -1; }
            /// Return the first index
        int getFirstIndex() const { return (size > 0) ? index : -1; }
            /// True if contains register index
        bool hasRegister(int registerIndex) const { return registerIndex >= index && registerIndex < index + size; }

        static RegisterRange makeInvalid() { return RegisterRange{ -1, 0 }; }
        static RegisterRange makeSingle(int index) { return RegisterRange{ int16_t(index), 1 }; }
        static RegisterRange makeRange(int index, int size) { return RegisterRange{ int16_t(index), uint16_t(size) }; }

        int16_t index;              ///< The base index
        uint16_t size;              ///< The amount of register indices
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
        RegisterRange registerRange;        /// Defines the registers for binding
    };

    struct Desc
    {
            /// Add a resource - assumed that the binding will match the Desc of the resource
        void addResource(BindingType bindingType, Resource* resource, const RegisterRange& registerRange);
            /// Add a sampler
        void addSampler(const SamplerDesc& desc, const RegisterRange& registerRange);
            /// Add a BufferResource
        void addBufferResource(BufferResource* resource, const RegisterRange& registerRange) { addResource(BindingType::Buffer, resource, registerRange); }
            /// Add a texture
        void addTextureResource(TextureResource* resource, const RegisterRange& registerRange) { addResource(BindingType::Texture, resource, registerRange); }
            /// Add combined texture a
        void addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const RegisterRange& registerRange);

            /// Returns the bind index, that has the bind flag, and indexes the specified register
        int findBindingIndex(Resource::BindFlag::Enum bindFlag, int registerIndex) const;

            /// Clear the contents
        void clear();

        Slang::List<Binding> m_bindings;                            ///< All of the bindings in order
        Slang::List<SamplerDesc> m_samplerDescs;                    ///< Holds the SamplerDesc for the binding - indexed by the descIndex member of Binding

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

    virtual ShaderProgram* createProgram(const ShaderProgram::Desc& desc) = 0;

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
