// render.h
#pragma once

//#include "shader-input-layout.h"

#include <float.h>

#include "../../slang-com-helper.h"

#include "../../source/core/slang-smart-pointer.h"
#include "../../source/core/slang-list.h"
#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-process-util.h"

#include "../../slang.h"

namespace gfx {

using Slang::RefObject;
using Slang::RefPtr;
using Slang::Dictionary;
using Slang::List;

using Slang::getHashCode;
using Slang::combineHash;

typedef SlangResult Result;

// Had to move here, because Options needs types defined here
typedef SlangInt Int;
typedef SlangUInt UInt;

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
    RayTracing,
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
    RayGeneration,
    Intersection,
    AnyHit,
    ClosestHit,
    Miss,
    Callable,
    CountOf,
};

enum class RendererType
{
    Unknown,
    DirectX11,
    DirectX12,
    OpenGl,
    Vulkan,
    CPU,
    CUDA,
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
    CPU,
    CUDA,
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
        char const* entryPointName;

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
        SlangStage  slangStage;
    };

    SourceInfo source;
    Slang::List<EntryPoint> entryPoints;

    Slang::List<Slang::String> globalSpecializationArgs;
    Slang::List<Slang::String> entryPointSpecializationArgs;

    Slang::List<Slang::CommandLine::Arg> compileArgs;
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

    R_UInt16,
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

// Needed for building on cygwin with gcc
#undef Always
#undef None

enum class ComparisonFunc : uint8_t
{
    Never           = 0,
    Less            = 0x01,
    Equal           = 0x02,
    LessEqual       = 0x03,
    Greater         = 0x04,
    NotEqual        = 0x05,
    GreaterEqual    = 0x06,
    Always          = 0x07,
};

enum class TextureFilteringMode
{
    Point,
    Linear,
};

enum class TextureAddressingMode
{
    Wrap,
    ClampToEdge,
    ClampToBorder,
    MirrorRepeat,
    MirrorOnce,
};

enum class TextureReductionOp
{
    Average,
    Comparison,
    Minimum,
    Maximum,
};

class SamplerState : public Slang::RefObject
{
public:
    struct Desc
    {
        TextureFilteringMode    minFilter       = TextureFilteringMode::Linear;
        TextureFilteringMode    magFilter       = TextureFilteringMode::Linear;
        TextureFilteringMode    mipFilter       = TextureFilteringMode::Linear;
        TextureReductionOp      reductionOp     = TextureReductionOp::Average;
        TextureAddressingMode   addressU        = TextureAddressingMode::Wrap;
        TextureAddressingMode   addressV        = TextureAddressingMode::Wrap;
        TextureAddressingMode   addressW        = TextureAddressingMode::Wrap;
        float                   mipLODBias      = 0.0f;
        uint32_t                maxAnisotropy   = 1;
        ComparisonFunc          comparisonFunc  = ComparisonFunc::Never;
        float                   borderColor[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
        float                   minLOD          = -FLT_MAX;
        float                   maxLOD          = FLT_MAX;
    };
};

enum class DescriptorSlotType
{
    Unknown,

    Sampler,
    CombinedImageSampler,
    SampledImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    DynamicUniformBuffer,
    DynamicStorageBuffer,
    InputAttachment,
    RootConstant,
    InlineUniformBlock,
    RayTracingAccelerationStructure,
};

class DescriptorSetLayout : public Slang::RefObject
{
public:
    struct SlotRangeDesc
    {
        DescriptorSlotType  type            = DescriptorSlotType::Unknown;
        UInt                count           = 1;

            /// The underlying API-specific binding/register to use for this slot range.
            ///
            /// A value of `-1` indicates that the implementation should
            /// automatically compute the binding/register to use
            /// based on the preceeding slot range(s).
            ///
            /// Some implementations do not have a concept of bindings/regsiters
            /// for slot ranges, and will ignore this field.
            ///
        Int                 binding         = -1;

        SlotRangeDesc()
        {}

        SlotRangeDesc(
            DescriptorSlotType  type,
            UInt                count = 1)
            : type(type)
            , count(count)
        {}
    };

    struct Desc
    {
        UInt                    slotRangeCount  = 0;
        SlotRangeDesc const*    slotRanges      = nullptr;
    };
};

class PipelineLayout : public Slang::RefObject
{
public:
    struct DescriptorSetDesc
    {
        DescriptorSetLayout*    layout          = nullptr;

            /// The underlying API-specific space/set number to use for this set.
            ///
            /// A value of `-1` indicates that the implementation should
            /// automatically compute the space/set to use basd on
            /// the preceeding set(s)
            ///
            /// Some implementations do not have a concept of space/set numbers
            /// for descriptor sets, and will ignore this field.
            ///
        Int                     space = -1;

        DescriptorSetDesc()
        {}

        DescriptorSetDesc(
            DescriptorSetLayout*    layout)
            : layout(layout)
        {}
    };

    struct Desc
    {
        UInt                        renderTargetCount   = 0;
        UInt                        descriptorSetCount  = 0;
        DescriptorSetDesc const*    descriptorSets      = nullptr;
    };
};

class ResourceView : public Slang::RefObject
{
public:
    enum class Type
    {
        Unknown,

        RenderTarget,
        DepthStencil,
        ShaderResource,
        UnorderedAccess,
    };

    struct Desc
    {
        Type    type;
        Format  format;
    };
};

class DescriptorSet : public Slang::RefObject
{
public:
    virtual void setConstantBuffer(UInt range, UInt index, BufferResource* buffer) = 0;
    virtual void setResource(UInt range, UInt index, ResourceView* view) = 0;
    virtual void setSampler(UInt range, UInt index, SamplerState* sampler) = 0;
    virtual void setCombinedTextureSampler(
        UInt range,
        UInt index,
        ResourceView*   textureView,
        SamplerState*   sampler) = 0;
    virtual void setRootConstants(
        UInt range,
        UInt offset,
        UInt size,
        void const* data) = 0;
};

struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
};

class ShaderObjectLayout : public Slang::RefObject
{};

class ShaderObject : public Slang::RefObject
{
public:
    ShaderObject* getObject(ShaderOffset const& offset)
    {
        ShaderObject* object = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getObject(offset, &object));
        return object;
    }

    virtual slang::TypeLayoutReflection* getElementTypeLayout() = 0;
    virtual Slang::Index getEntryPointCount() = 0;
    virtual ShaderObject* getEntryPoint(Slang::Index index) = 0;
    virtual SlangResult setData(ShaderOffset const& offset, void const* data, size_t size) = 0;
    virtual SlangResult getObject(ShaderOffset const& offset, ShaderObject** object) = 0;
    virtual SlangResult setObject(ShaderOffset const& offset, ShaderObject* object) = 0;
    virtual SlangResult setResource(ShaderOffset const& offset, ResourceView* resourceView) = 0;
    virtual SlangResult setSampler(ShaderOffset const& offset, SamplerState* sampler) = 0;
    virtual SlangResult setCombinedTextureSampler(
        ShaderOffset const& offset, ResourceView* textureView, SamplerState* sampler) = 0;
};

enum class StencilOp : uint8_t
{
    Keep,
    Zero,
    Replace,
    IncrementSaturate,
    DecrementSaturate,
    Invert,
    IncrementWrap,
    DecrementWrap,
};

enum class FillMode : uint8_t
{
    Solid,
    Wireframe,
};

enum class CullMode : uint8_t
{
    None,
    Front,
    Back,
};

enum class FrontFaceMode : uint8_t
{
    CounterClockwise,
    Clockwise,
};

struct DepthStencilOpDesc
{
    StencilOp       stencilFailOp       = StencilOp::Keep;
    StencilOp       stencilDepthFailOp  = StencilOp::Keep;
    StencilOp       stencilPassOp       = StencilOp::Keep;
    ComparisonFunc  stencilFunc         = ComparisonFunc::Always;
};

struct DepthStencilDesc
{
    bool            depthTestEnable     = true;
    bool            depthWriteEnable    = true;
    ComparisonFunc  depthFunc           = ComparisonFunc::Less;

    bool                stencilEnable       = false;
    uint32_t            stencilReadMask     = 0xFFFFFFFF;
    uint32_t            stencilWriteMask    = 0xFFFFFFFF;
    DepthStencilOpDesc  frontFace;
    DepthStencilOpDesc  backFace;

    uint32_t stencilRef = 0;
};

struct RasterizerDesc
{
    FillMode        fillMode                = FillMode::Solid;
    CullMode        cullMode                = CullMode::Back;
    FrontFaceMode   frontFace               = FrontFaceMode::CounterClockwise;
    int32_t         depthBias               = 0;
    float           depthBiasClamp          = 0.0f;
    float           slopeScaledDepthBias    = 0.0f;
    bool            depthClipEnable         = true;
    bool            scissorEnable           = false;
    bool            multisampleEnable       = false;
    bool            antialiasedLineEnable   = false;
};

enum class LogicOp
{
    NoOp,
};

enum class BlendOp
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

enum class BlendFactor
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSaturate,
    BlendColor,
    InvBlendColor,
    SecondarySrcColor,
    InvSecondarySrcColor,
    SecondarySrcAlpha,
    InvSecondarySrcAlpha,
};

namespace RenderTargetWriteMask
{
    typedef uint8_t Type;
    enum
    {
        EnableNone  = 0,
        EnableRed   = 0x01,
        EnableGreen = 0x02,
        EnableBlue  = 0x04,
        EnableAlpha = 0x08,
        EnableAll   = 0x0F,
    };
};
typedef RenderTargetWriteMask::Type RenderTargetWriteMaskT;

struct AspectBlendDesc
{
    BlendFactor     srcFactor   = BlendFactor::One;
    BlendFactor     dstFactor   = BlendFactor::Zero;
    BlendOp         op          = BlendOp::Add;
};

struct TargetBlendDesc
{
    AspectBlendDesc color;
    AspectBlendDesc alpha;

    LogicOp                 logicOp     = LogicOp::NoOp;
    RenderTargetWriteMaskT  writeMask   = RenderTargetWriteMask::EnableAll;
};

struct BlendDesc
{
    TargetBlendDesc const*  targets     = nullptr;
    UInt                    targetCount = 0;

    bool alphaToCoverateEnable  = false;
};

struct GraphicsPipelineStateDesc
{
    ShaderProgram*      program;
    // Application should set either pipelineLayout or rootShaderObjectLayout, but not both.
    PipelineLayout* pipelineLayout = nullptr;
    // Application should set either pipelineLayout or rootShaderObjectLayout, but not both.
    ShaderObjectLayout* rootShaderObjectLayout = nullptr;
    InputLayout*        inputLayout;
    UInt                framebufferWidth;
    UInt                framebufferHeight;
    UInt                renderTargetCount = 0; // Not used if rootShaderObjectLayout is set.
    DepthStencilDesc    depthStencil;
    RasterizerDesc      rasterizer;
    BlendDesc           blend;
};

struct ComputePipelineStateDesc
{
    ShaderProgram*  program;
    PipelineLayout* pipelineLayout = nullptr;
    ShaderObjectLayout* rootShaderObjectLayout = nullptr;
};

class PipelineState : public Slang::RefObject
{
public:
};

struct ScissorRect
{
    Int minX;
    Int minY;
    Int maxX;
    Int maxY;
};

struct Viewport
{
    float originX = 0.0f;
    float originY = 0.0f;
    float extentX = 0.0f;
    float extentY = 0.0f;
    float minZ    = 0.0f;
    float maxZ    = 1.0f;
};

class IRenderer: public ISlangUnknown
{
public:

    struct Desc
    {
        int width = 0;                                  ///< Width in pixels
        int height = 0;                                 ///< height in pixels
        Slang::String adapter;                          ///< Name to identify the adapter to use
        Slang::List<Slang::String> requiredFeatures;    ///< List of required feature names. 
        int nvapiExtnSlot = -1;                         ///< The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
    };

        // Will return with SLANG_E_NOT_AVAILABLE if NVAPI can't be initialized and nvapiExtnSlot >= 0
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc, void* inWindowHandle) = 0;

    bool hasFeature(const Slang::UnownedStringSlice& feature) { return getFeatures().indexOf(Slang::String(feature)) != Slang::Index(-1); }
    virtual SLANG_NO_THROW const Slang::List<Slang::String>& SLANG_MCALL getFeatures() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL presentFrame() = 0;

    virtual SLANG_NO_THROW TextureResource::Desc SLANG_MCALL getSwapChainTextureDesc() = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        Resource::Usage initialUsage,
        const TextureResource::Desc& desc,
        const TextureResource::Data* initData,
        TextureResource** outResource) = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    inline SLANG_NO_THROW RefPtr<TextureResource> SLANG_MCALL createTextureResource(
        Resource::Usage initialUsage,
        const TextureResource::Desc& desc,
        const TextureResource::Data* initData = nullptr)
    {
        RefPtr<TextureResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createTextureResource(initialUsage, desc, initData, resource.writeRef()));
        return resource;
    }

        /// Create a buffer resource
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        Resource::Usage initialUsage,
        const BufferResource::Desc& desc,
        const void* initData,
        BufferResource** outResource) = 0;

    inline SLANG_NO_THROW RefPtr<BufferResource> SLANG_MCALL createBufferResource(
        Resource::Usage initialUsage,
        const BufferResource::Desc& desc,
        const void* initData = nullptr)
    {
        RefPtr<BufferResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createBufferResource(initialUsage, desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler) = 0;

    inline RefPtr<SamplerState> createSamplerState(SamplerState::Desc const& desc)
    {
        RefPtr<SamplerState> sampler;
        SLANG_RETURN_NULL_ON_FAIL(createSamplerState(desc, sampler.writeRef()));
        return sampler;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView) = 0;

    inline RefPtr<ResourceView> createTextureView(TextureResource* texture, ResourceView::Desc const& desc)
    {
        RefPtr<ResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createTextureView(texture, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView) = 0;

    inline RefPtr<ResourceView> createBufferView(BufferResource* buffer, ResourceView::Desc const& desc)
    {
        RefPtr<ResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createBufferView(buffer, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements, UInt inputElementCount, InputLayout** outLayout) = 0;

    inline RefPtr<InputLayout> createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount)
    {
        RefPtr<InputLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createInputLayout(inputElements, inputElementCount, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout) = 0;

    inline RefPtr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc)
    {
        RefPtr<DescriptorSetLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createDescriptorSetLayout(desc, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayout** outLayout) = 0;

    inline RefPtr<ShaderObjectLayout> createShaderObjectLayout(slang::TypeLayoutReflection* typeLayout)
    {
        RefPtr<ShaderObjectLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObjectLayout(typeLayout, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObjectLayout(slang::ProgramLayout* layout, ShaderObjectLayout** outLayout) = 0;

    inline RefPtr<ShaderObjectLayout> createRootShaderObjectLayout(slang::ProgramLayout* layout)
    {
        RefPtr<ShaderObjectLayout> result;
        SLANG_RETURN_NULL_ON_FAIL(createRootShaderObjectLayout(layout, result.writeRef()));
        return result;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) = 0;

    inline RefPtr<ShaderObject> createShaderObject(ShaderObjectLayout* layout)
    {
        RefPtr<ShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObject(layout, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(ShaderObjectLayout* layout, ShaderObject** outObject) = 0;

    inline RefPtr<ShaderObject> createRootShaderObject(ShaderObjectLayout* layout)
    {
        RefPtr<ShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createRootShaderObject(layout, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL bindRootShaderObject(PipelineType pipelineType, ShaderObject* object) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout) = 0;

    inline RefPtr<PipelineLayout> createPipelineLayout(const PipelineLayout::Desc& desc)
    {
        RefPtr<PipelineLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createPipelineLayout(desc, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet) = 0;

    inline RefPtr<DescriptorSet> createDescriptorSet(DescriptorSetLayout* layout)
    {
        RefPtr<DescriptorSet> descriptorSet;
        SLANG_RETURN_NULL_ON_FAIL(createDescriptorSet(layout, descriptorSet.writeRef()));
        return descriptorSet;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram) = 0;

    inline RefPtr<ShaderProgram> createProgram(const ShaderProgram::Desc& desc)
    {
        RefPtr<ShaderProgram> program;
        SLANG_RETURN_NULL_ON_FAIL(createProgram(desc, program.writeRef()));
        return program;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc&    desc,
        PipelineState**                     outState) = 0;

    inline RefPtr<PipelineState> createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc)
    {
        RefPtr<PipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createGraphicsPipelineState(desc, state.writeRef()));
        return state;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc&    desc,
        PipelineState**                     outState) = 0;

    inline RefPtr<PipelineState> createComputePipelineState(
        const ComputePipelineStateDesc& desc)
    {
        RefPtr<PipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createComputePipelineState(desc, state.writeRef()));
        return state;
    }

        /// Captures the back buffer and stores the result in surfaceOut. If the surface contains data - it will either be overwritten (if same size and format), or freed and a re-allocated.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL captureScreenSurface(Surface& surfaceOut) = 0;

    virtual SLANG_NO_THROW void* SLANG_MCALL map(BufferResource* buffer, MapFlavor flavor) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL unmap(BufferResource* buffer) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        PipelineLayout* layout,
        UInt index,
        DescriptorSet* descriptorSet) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        BufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) = 0;
    inline void setVertexBuffer(UInt slot, BufferResource* buffer, UInt stride, UInt offset = 0);

    virtual SLANG_NO_THROW void SLANG_MCALL setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset = 0) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setDepthStencilTarget(ResourceView* depthStencilView) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setViewports(UInt count, Viewport const* viewports) = 0;
    inline void setViewport(Viewport const& viewport)
    {
        setViewports(1, &viewport);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setScissorRects(UInt count, ScissorRect const* rects) = 0;
    inline void setScissorRect(ScissorRect const& rect)
    {
        setScissorRects(1, &rect);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(PipelineType pipelineType, PipelineState* state) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) = 0;

        /// Commit any buffered state changes or draw calls.
        /// presentFrame will commitAll implicitly before doing a present
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() = 0;
        /// Blocks until Gpu work is complete
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() = 0;

        /// Get the type of this renderer
    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const = 0;
};

#define SLANG_UUID_IRenderer                                                             \
    {                                                                                    \
          0x715bdf26, 0x5135, 0x11eb, { 0xAE, 0x93, 0x02, 0x42, 0xAC, 0x13, 0x00, 0x02 } \
    }

// ----------------------------------------------------------------------------------------
inline void IRenderer::setVertexBuffer(UInt slot, BufferResource* buffer, UInt stride, UInt offset)
{
    setVertexBuffers(slot, 1, &buffer, &stride, &offset);
}

/// Functions that are around Renderer and it's types
struct RendererUtil
{
    typedef SlangResult (*CreateFunc)(IRenderer** outRenderer);

        /// Gets the size in bytes of a Format type. Returns 0 if a size is not defined/invalid
    SLANG_FORCE_INLINE static size_t getFormatSize(Format format) { return s_formatSize[int(format)]; }
        /// Given a renderer type, gets a projection style
    static ProjectionStyle getProjectionStyle(RendererType type);

        /// Given the projection style returns an 'identity' matrix, which ensures x,y mapping to pixels is the same on all targets
    static void getIdentityProjection(ProjectionStyle style, float projMatrix[16]);

        /// Get the binding style from the type
    static BindingStyle getBindingStyle(RendererType type) { return s_rendererTypeToBindingStyle[int(type)]; }

        /// Get as text
    static Slang::UnownedStringSlice toText(RendererType type);

        /// Given a type returns a function that can construct it, or nullptr if there isn't one
    static CreateFunc getCreateFunc(RendererType type);

    private:
    static void compileTimeAsserts();
    static const uint8_t s_formatSize[]; // Maps Format::XXX to a size in bytes;
    static const BindingStyle s_rendererTypeToBindingStyle[];           ///< Maps a RendererType to a BindingStyle
};

} // renderer_test
