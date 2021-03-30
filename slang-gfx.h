// render.h
#pragma once

#include <float.h>
#include <assert.h>

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"


#if defined(SLANG_GFX_DYNAMIC)
#    if defined(_MSC_VER)
#        ifdef SLANG_GFX_DYNAMIC_EXPORT
#            define SLANG_GFX_API SLANG_DLL_EXPORT
#        else
#            define SLANG_GFX_API __declspec(dllimport)
#        endif
#    else
// TODO: need to consider compiler capabilities
//#     ifdef SLANG_DYNAMIC_EXPORT
#        define SLANG_GFX_API SLANG_DLL_EXPORT
//#     endif
#    endif
#endif

#ifndef SLANG_GFX_API
#    define SLANG_GFX_API
#endif

namespace gfx {

using Slang::ComPtr;

typedef SlangResult Result;

// Had to move here, because Options needs types defined here
typedef SlangInt Int;
typedef SlangUInt UInt;

// Declare opaque type
class IInputLayout: public ISlangUnknown
{
};
#define SLANG_UUID_IInputLayout                                                         \
    {                                                                                  \
        0x45223711, 0xa84b, 0x455c, { 0xbe, 0xfa, 0x49, 0x37, 0x42, 0x1e, 0x8e, 0x2e } \
    }

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
    Amplification,
    Mesh,
    CountOf,
};

enum class DeviceType
{
    Unknown,
    Default,
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

class IShaderProgram: public ISlangUnknown
{
public:
    struct Desc
    {
        PipelineType        pipelineType;
        slang::IComponentType*  slangProgram;
    };
};
#define SLANG_UUID_IShaderProgram                                                       \
    {                                                                                  \
        0x9d32d0ad, 0x915c, 0x4ffd, { 0x91, 0xe2, 0x50, 0x85, 0x54, 0xa0, 0x4a, 0x76 } \
    }

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
    BGRA_Unorm_UInt8,

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

enum class PrimitiveType
{
    Point, Line, Triangle, Patch
};

enum class PrimitiveTopology
{
    TriangleList,
};

class IResource: public ISlangUnknown
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
        Unknown = 0,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        StreamOutput,
        RenderTarget,
        DepthRead,
        DepthWrite,
        UnorderedAccess,
        PixelShaderResource,
        NonPixelShaderResource,
        ShaderResource,
        GenericRead,
        CopySource,
        CopyDest,
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

    inline static BindFlag::Enum getDefaultBindFlagsFromUsage(IResource::Usage usage)
    {
        switch (usage)
        {
        case Usage::VertexBuffer:
            return BindFlag::VertexBuffer;
        case Usage::IndexBuffer:
            return BindFlag::IndexBuffer;
        case Usage::ConstantBuffer:
            return BindFlag::ConstantBuffer;
        case Usage::StreamOutput:
            return BindFlag::StreamOutput;
        case Usage::RenderTarget:
            return BindFlag::RenderTarget;
        case Usage::DepthRead:
        case Usage::DepthWrite:
            return BindFlag::DepthStencil;
        case Usage::UnorderedAccess:
            return BindFlag::Enum(BindFlag::UnorderedAccess | BindFlag::PixelShaderResource |
                BindFlag::NonPixelShaderResource);
        case Usage::PixelShaderResource:
            return BindFlag::PixelShaderResource;
        case Usage::NonPixelShaderResource:
            return BindFlag::NonPixelShaderResource;
        case Usage::ShaderResource:
        case Usage::GenericRead:
            return BindFlag::Enum(
                BindFlag::PixelShaderResource |
                BindFlag::NonPixelShaderResource);
        case Usage::CopySource:
            return BindFlag::Enum(0);
        case Usage::CopyDest:
            return BindFlag::Enum(0);
        default:
            return BindFlag::Enum(-1);
        }
    }

    virtual SLANG_NO_THROW Type SLANG_MCALL getType() = 0;
};
#define SLANG_UUID_IResource                                                           \
    {                                                                                  \
        0xa0e39f34, 0x8398, 0x4522, { 0x95, 0xc2, 0xeb, 0xc0, 0xf9, 0x84, 0xef, 0x3f } \
    }

class IBufferResource: public IResource
{
public:
    struct Desc: public DescBase
    {
        void init(size_t sizeInBytesIn)
        {
            sizeInBytes = sizeInBytesIn;
            elementSize = 0;
            format = Format::Unknown;
        }
        void setDefaults(Usage initialUsage)
        {
            if (bindFlags == 0)
            {
                bindFlags = getDefaultBindFlagsFromUsage(initialUsage);
            }
        }
        size_t sizeInBytes;     ///< Total size in bytes
        int elementSize;        ///< Get the element stride. If > 0, this is a structured buffer
        Format format;
    };
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() = 0;
};
#define SLANG_UUID_IBufferResource                                                     \
    {                                                                                  \
        0x1b274efe, 0x5e37, 0x492b, { 0x82, 0x6e, 0x7e, 0xe7, 0xe8, 0xf5, 0xa4, 0x9b } \
    }

template <typename T> T _slang_gfx_max(T v0, T v1) { return v0 > v1 ? v0 : v1; }

static inline unsigned int _slang_gfx_ones32(unsigned int x)
{
    /* 32-bit recursive reduction using SWAR...
            but first step is mapping 2-bit values
            into sum of 2 1-bit values in sneaky way
    */
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return (x & 0x0000003f);
}

static inline unsigned int _slang_gfx_log2Floor(unsigned int x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (_slang_gfx_ones32(x >> 1));
}

struct DepthStencilClearValue
{
    float depth = 1.0f;
    uint32_t stencil = 0;
};
union ColorClearValue
{
    float floatValues[4];
    uint32_t uintValues[4];
};
struct ClearValue
{
    ColorClearValue color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    DepthStencilClearValue depthStencil;
};

class ITextureResource: public IResource
{
public:
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
        int calcMaxDimension(Type type) const
        {
            switch (type)
            {
            case IResource::Type::Texture1D:
                return this->width;
            case IResource::Type::Texture3D:
                return _slang_gfx_max(_slang_gfx_max(this->width, this->height), this->depth);
            case IResource::Type::TextureCube: // fallthru
            case IResource::Type::Texture2D:
                {
                    return _slang_gfx_max(this->width, this->height);
                }
            default:
                return 0;
            }
        }

        SLANG_FORCE_INLINE static int calcMipSize(int width, int mipLevel)
        {
            width = width >> mipLevel;
            return width > 0 ? width : 1;
        }
            /// Given a size, calculates the size at a mip level
        Size calcMipSize(int mipLevel) const
        {
            Size size;
            size.width = calcMipSize(this->width, mipLevel);
            size.height = calcMipSize(this->height, mipLevel);
            size.depth = calcMipSize(this->depth, mipLevel);
            return size;
        }

        int width;              ///< Width in pixels
        int height;             ///< Height in pixels (if 2d or 3d)
        int depth;              ///< Depth (if 3d)
    };

    struct Desc: public DescBase
    {
            /// Initialize with default values
        void init(Type typeIn)
        {
            this->type = typeIn;
            this->size.init();

            this->format = Format::Unknown;
            this->arraySize = 0;
            this->numMipLevels = 0;
            this->sampleDesc.init();

            this->bindFlags = 0;
            this->cpuAccessFlags = 0;
        }
            /// Initialize different dimensions. For cubemap, use init2D
        void init1D(Format formatIn, int widthIn, int numMipMapsIn = 0)
        {
            this->type = Type::Texture1D;
            this->size.init(widthIn);

            this->format = formatIn;
            this->arraySize = 0;
            this->numMipLevels = numMipMapsIn;
            this->sampleDesc.init();

            this->bindFlags = 0;
            this->cpuAccessFlags = 0;
        }

        void init2D(Type typeIn, Format formatIn, int widthIn, int heightIn, int numMipMapsIn = 0)
        {
            assert(typeIn == Type::Texture2D || typeIn == Type::TextureCube);

            this->type = typeIn;
            this->size.init(widthIn, heightIn);

            this->format = formatIn;
            this->arraySize = 0;
            this->numMipLevels = numMipMapsIn;
            this->sampleDesc.init();

            this->bindFlags = 0;
            this->cpuAccessFlags = 0;
        }

        void init3D(Format formatIn, int widthIn, int heightIn, int depthIn, int numMipMapsIn = 0)
        {
            this->type = Type::Texture3D;
            this->size.init(widthIn, heightIn, depthIn);

            this->format = formatIn;
            this->arraySize = 0;
            this->numMipLevels = numMipMapsIn;
            this->sampleDesc.init();

            this->bindFlags = 0;
            this->cpuAccessFlags = 0;
        }

            /// Given the type, calculates the number of mip maps. 0 on error
        int calcNumMipLevels() const
        {
            const int maxDimensionSize = this->size.calcMaxDimension(type);
            return (maxDimensionSize > 0) ? (_slang_gfx_log2Floor(maxDimensionSize) + 1) : 0;
        }
            /// Calculate the total number of sub resources. 0 on error.
        int calcNumSubResources() const
        {
            const int numMipMaps =
                (this->numMipLevels > 0) ? this->numMipLevels : calcNumMipLevels();
            const int arrSize = (this->arraySize > 0) ? this->arraySize : 1;

            switch (type)
            {
            case IResource::Type::Texture1D:
            case IResource::Type::Texture2D:
            case IResource::Type::Texture3D:
                {
                    return numMipMaps * arrSize;
                }
            case IResource::Type::TextureCube:
                {
                    // There are 6 faces to a cubemap
                    return numMipMaps * arrSize * 6;
                }
            default:
                return 0;
            }
        }

            /// Calculate the effective array size - in essence the amount if mip map sets needed.
            /// In practice takes into account if the arraySize is 0 (it's not an array, but it will still have at least one mip set)
            /// and if the type is a cubemap (multiplies the amount of mip sets by 6)
        int calcEffectiveArraySize() const
        {
            const int arrSize = (this->arraySize > 0) ? this->arraySize : 1;

            switch (type)
            {
            case IResource::Type::Texture1D: // fallthru
            case IResource::Type::Texture2D:
                {
                    return arrSize;
                }
            case IResource::Type::TextureCube:
                return arrSize * 6;
            case IResource::Type::Texture3D:
                return 1;
            default:
                return 0;
            }
        }

            /// Use type to fix the size values (and array size).
            /// For example a 1d texture, should have height and depth set to 1.
        void fixSize()
        {
            switch (type)
            {
            case IResource::Type::Texture1D:
                {
                    this->size.height = 1;
                    this->size.depth = 1;
                    break;
                }
            case IResource::Type::TextureCube:
            case IResource::Type::Texture2D:
                {
                    this->size.depth = 1;
                    break;
                }
            case IResource::Type::Texture3D:
                {
                    // Can't have an array
                    this->arraySize = 0;
                    break;
                }
            default:
                break;
            }
        }

            /// Set up default parameters based on type and usage
        void setDefaults(Usage usage)
        {
            this->initialUsage = usage;
            fixSize();
            if (this->bindFlags == 0)
            {
                this->bindFlags = getDefaultBindFlagsFromUsage(initialUsage);
            }
            if (this->numMipLevels <= 0)
            {
                this->numMipLevels = calcNumMipLevels();
            }
        }

        Size size;

        int arraySize;          ///< Array size

        int numMipLevels;       ///< Number of mip levels - if 0 will create all mip levels
        Format format;          ///< The resources format
        SampleDesc sampleDesc;  ///< How the resource is sampled
        ClearValue optimalClearValue;
        Usage initialUsage;
    };

        /// Data for a single subresource of a texture.
        ///
        /// Each subresource is a tensor with `1 <= rank <= 3`,
        /// where the rank is deterined by the base shape of the
        /// texture (Buffer, 1D, 2D, 3D, or Cube). For the common
        /// case of a 2D texture, `rank == 2` and each subresource
        /// is a 2D image.
        ///
        /// Subresource tensors must be stored in a row-major layout,
        /// so that the X axis strides over texels, the Y axis strides
        /// over 1D rows of texels, and the Z axis strides over 2D
        /// "layers" of texels.
        ///
        /// For a texture with multiple mip levels or array elements,
        /// each mip level and array element is stores as a distinct
        /// subresource. When indexing into an array of subresources,
        /// the index of a subresoruce for mip level `m` and array
        /// index `a` is `m + a*mipLevelCount`.
        ///
    struct SubresourceData
    {
            /// Pointer to texel data for the subresource tensor.
        void const* data;

            /// Stride in bytes between rows of the subresource tensor.
            ///
            /// This is the number of bytes to add to a pointer to a texel
            /// at (X,Y,Z) to get to a texel at (X,Y+1,Z).
            ///
            /// Devices may not support all possible values for `strideY`.
            /// In particular, they may only support strictly positive strides.
            ///
        int64_t     strideY;

            /// Stride in bytes between layers of the subresource tensor.
            ///
            /// This is the number of bytes to add to a pointer to a texel
            /// at (X,Y,Z) to get to a texel at (X,Y,Z+1).
            ///
            /// Devices may not support all possible values for `strideZ`.
            /// In particular, they may only support strictly positive strides.
            ///
        int64_t     strideZ;
    };

    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() = 0;
};
#define SLANG_UUID_ITextureResource                                                    \
    {                                                                                  \
        0xcf88a31c, 0x6187, 0x46c5, { 0xa4, 0xb7, 0xeb, 0x58, 0xc7, 0x33, 0x40, 0x17 } \
    } 

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

class ISamplerState : public ISlangUnknown
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
#define SLANG_UUID_ISamplerState                                                        \
    {                                                                                  \
        0x8b8055df, 0x9377, 0x401d, { 0x91, 0xff, 0x3f, 0xa3, 0xbf, 0x66, 0x64, 0xf4 } \
    }

class IResourceView : public ISlangUnknown
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

    struct RenderTargetDesc
    {
        // The resource shape of this render target view.
        IResource::Type shape;
        uint32_t mipSlice;
        uint32_t arrayIndex;
        uint32_t arraySize;
        uint32_t planeIndex;
    };

    struct Desc
    {
        Type    type;
        Format  format;

        // Fields for `RenderTarget` and `DepthStencil` views.
        RenderTargetDesc renderTarget;
    };
};
#define SLANG_UUID_IResourceView                                                       \
    {                                                                                 \
        0x7b6c4926, 0x884, 0x408c, { 0xad, 0x8a, 0x50, 0x3a, 0x8e, 0x23, 0x98, 0xa4 } \
    }

struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
};

class IShaderObject : public ISlangUnknown
{
public:
    SLANG_NO_THROW ComPtr<IShaderObject> SLANG_MCALL getObject(ShaderOffset const& offset)
    {
        ComPtr<IShaderObject> object = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getObject(offset, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() = 0;
    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() = 0;

    ComPtr<IShaderObject> getEntryPoint(UInt index)
    {
        ComPtr<IShaderObject> entryPoint = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getEntryPoint(index, entryPoint.writeRef()));
        return entryPoint;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** entryPoint) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** object) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) = 0;
};
#define SLANG_UUID_IShaderObject                                                       \
    {                                                                                 \
        0xc1fa997e, 0x5ca2, 0x45ae, { 0x9b, 0xcb, 0xc4, 0x35, 0x9e, 0x85, 0x5, 0x85 } \
    }


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
    uint32_t             stencilReadMask     = 0xFFFFFFFF;
    uint32_t             stencilWriteMask    = 0xFFFFFFFF;
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

class IFramebufferLayout : public ISlangUnknown
{
public:
    struct AttachmentLayout
    {
        Format format;
        int sampleCount;
    };
    struct Desc
    {
        uint32_t renderTargetCount;
        AttachmentLayout* renderTargets;
        AttachmentLayout* depthStencil;
    };
};
#define SLANG_UUID_IFramebufferLayout                                                \
    {                                                                                \
        0xa838785, 0xc13a, 0x4832, { 0xad, 0x88, 0x64, 0x6, 0xb5, 0x4b, 0x5e, 0xba } \
    }

struct GraphicsPipelineStateDesc
{
    IShaderProgram*      program = nullptr;

    IInputLayout*       inputLayout = nullptr;
    IFramebufferLayout* framebufferLayout = nullptr;
    PrimitiveType       primitiveType = PrimitiveType::Triangle;
    DepthStencilDesc    depthStencil;
    RasterizerDesc      rasterizer;
    BlendDesc           blend;
};

struct ComputePipelineStateDesc
{
    IShaderProgram*  program;
};

class IPipelineState : public ISlangUnknown
{
};
#define SLANG_UUID_IPipelineState                                                      \
    {                                                                                 \
        0xca7e57d, 0x8a90, 0x44f3, { 0xbd, 0xb1, 0xfe, 0x9b, 0x35, 0x3f, 0x5a, 0x72 } \
    }


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

class IFramebuffer : public ISlangUnknown
{
public:
    struct Desc
    {
        uint32_t renderTargetCount;
        IResourceView* const* renderTargetViews;
        IResourceView* depthStencilView;
        IFramebufferLayout* layout;
    };
};
#define SLANG_UUID_IFrameBuffer                                                       \
    {                                                                                 \
        0xf0c0d9a, 0x4ef3, 0x4e18, { 0x9b, 0xa9, 0x34, 0x60, 0xea, 0x69, 0x87, 0x95 } \
    }

struct WindowHandle
{
    enum class Type
    {
        Unknown,
        Win32Handle,
        XLibHandle,
    };
    Type type;
    intptr_t handleValues[2];
    static WindowHandle FromHwnd(void* hwnd)
    {
        WindowHandle handle = {};
        handle.type = WindowHandle::Type::Win32Handle;
        handle.handleValues[0] = (intptr_t)(hwnd);
        return handle;
    }
    static WindowHandle FromXWindow(void* xdisplay, uint32_t xwindow)
    {
        WindowHandle handle = {};
        handle.type = WindowHandle::Type::XLibHandle;
        handle.handleValues[0] = (intptr_t)(xdisplay);
        handle.handleValues[1] = xwindow;
        return handle;
    }
};

enum class ResourceState
{
    Undefined,
    ShaderResource,
    UnorderedAccess,
    RenderTarget,
    DepthRead,
    DepthWrite,
    Present,
    CopySource,
    CopyDestination,
    ResolveSource,
    ResolveDestination,
};

struct FaceMask
{
    enum Enum
    {
        Front = 1, Back = 2
    };
};

class IRenderPassLayout : public ISlangUnknown
{
public:
    enum class AttachmentLoadOp
    {
        Load, Clear, DontCare
    };
    enum class AttachmentStoreOp
    {
        Store, DontCare
    };
    struct AttachmentAccessDesc
    {
        AttachmentLoadOp loadOp;
        AttachmentLoadOp stencilLoadOp;
        AttachmentStoreOp storeOp;
        AttachmentStoreOp stencilStoreOp;
        ResourceState initialState;
        ResourceState finalState;
    };
    struct Desc
    {
        IFramebufferLayout* framebufferLayout;
        uint32_t renderTargetCount;
        AttachmentAccessDesc* renderTargetAccess;
        AttachmentAccessDesc* depthStencilAccess;
    };
};
#define SLANG_UUID_IRenderPassLayout                                                   \
    {                                                                                  \
        0xdaab0b1a, 0xf45d, 0x4ae9, { 0xbf, 0x2c, 0xe0, 0xbb, 0x76, 0x7d, 0xfa, 0xd1 } \
    }

class ICommandEncoder : public ISlangUnknown
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() = 0;
};
#define SLANG_UUID_ICommandEncoder                                                     \
    {                                                                                  \
        0xbd0717f8, 0xc4a7, 0x4603, { 0x94, 0xd4, 0x6f, 0x8f, 0x95, 0x16, 0x91, 0x47 } \
    }

class IRenderCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        bindRootShaderObject(IShaderObject* object) = 0;

    virtual SLANG_NO_THROW void
        SLANG_MCALL setViewports(uint32_t count, const Viewport* viewports) = 0;
    virtual SLANG_NO_THROW void
        SLANG_MCALL setScissorRects(uint32_t count, const ScissorRect* scissors) = 0;

    /// Sets the viewport, and sets the scissor rect to match the viewport.
    inline void setViewportAndScissor(Viewport const& viewport)
    {
        setViewports(1, &viewport);
        ScissorRect rect = {};
        rect.maxX = static_cast<gfx::Int>(viewport.extentX);
        rect.maxY = static_cast<gfx::Int>(viewport.extentY);
        setScissorRects(1, &rect);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) = 0;
    inline void setVertexBuffer(UInt slot, IBufferResource* buffer, UInt stride, UInt offset = 0)
    {
        setVertexBuffers(slot, 1, &buffer, &stride, &offset);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) = 0;
};
#define SLANG_UUID_IRenderCommandEncoder                                               \
    {                                                                                  \
        0x39417cf7, 0x8d97, 0x43a9, { 0xbb, 0x9f, 0x2f, 0x35, 0xe9, 0x11, 0xd0, 0x42 } \
    }

class IComputeCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
        bindRootShaderObject(IShaderObject* object) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) = 0;
};
#define SLANG_UUID_IComputeCommandEncoder                                              \
    {                                                                                  \
        0x65400452, 0xc877, 0x478f, { 0x91, 0x7d, 0x48, 0xd5, 0x41, 0x6f, 0x39, 0xab } \
    }

class IResourceCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) = 0;
};
#define SLANG_UUID_IResourceCommandEncoder                                             \
    {                                                                                  \
        0x5fe87643, 0x7ad7, 0x4177, { 0x8b, 0xd1, 0xd7, 0x84, 0xad, 0xcf, 0x3d, 0xce } \
    }

class ICommandBuffer : public ISlangUnknown
{
public:
    // Only one encoder may be open at a time. User must call `ICommandEncoder::endEncoding`
    // before calling other `encode*Commands` methods.
    // Once `endEncoding` is called, the `ICommandEncoder` object becomes obsolete and is
    // invalid for further use. To continue recording, the user must request a new encoder
    // object by calling one of the `encode*Commands` methods again.
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) = 0;
    ComPtr<IRenderCommandEncoder>
        encodeRenderCommands(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
    {
        ComPtr<IRenderCommandEncoder> result;
        encodeRenderCommands(renderPass, framebuffer, result.writeRef());
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) = 0;
    ComPtr<IComputeCommandEncoder> encodeComputeCommands()
    {
        ComPtr<IComputeCommandEncoder> result;
        encodeComputeCommands(result.writeRef());
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) = 0;
    ComPtr<IResourceCommandEncoder> encodeResourceCommands()
    {
        ComPtr<IResourceCommandEncoder> result;
        encodeResourceCommands(result.writeRef());
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL close() = 0;
};
#define SLANG_UUID_ICommandBuffer                                                      \
    {                                                                                  \
        0x5d56063f, 0x91d4, 0x4723, { 0xa7, 0xa7, 0x7a, 0x15, 0xaf, 0x93, 0xeb, 0x48 } \
    }

class ICommandQueue : public ISlangUnknown
{
public:
    enum class QueueType
    {
        Graphics
    };
    struct Desc
    {
        QueueType type;
    };
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL
        executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers) = 0;
    inline void executeCommandBuffer(ICommandBuffer* commandBuffer)
    {
        executeCommandBuffers(1, &commandBuffer);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL wait() = 0;
};
#define SLANG_UUID_ICommandQueue                                                    \
    {                                                                               \
        0x14e2bed0, 0xad0, 0x4dc8, { 0xb3, 0x41, 0x6, 0x3f, 0xe7, 0x2d, 0xbf, 0xe } \
    }

class ITransientResourceHeap : public ISlangUnknown
{
public:
    struct Desc
    {
        size_t constantBufferSize;
    };
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() = 0;

    // Command buffers are one-time use. Once it is submitted to the queue via
    // `executeCommandBuffers` a command buffer is no longer valid to be used any more. Command
    // buffers must be closed before submission. The current D3D12 implementation has a limitation
    // that only one command buffer maybe recorded at a time. User must finish recording a command
    // buffer before creating another command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) = 0;
    inline ComPtr<ICommandBuffer> createCommandBuffer()
    {
        ComPtr<ICommandBuffer> result;
        SLANG_RETURN_NULL_ON_FAIL(createCommandBuffer(result.writeRef()));
        return result;
    }
};
#define SLANG_UUID_ITransientResourceHeap                                             \
    {                                                                                 \
        0xcd48bd29, 0xee72, 0x41b8, { 0xbc, 0xff, 0xa, 0x2b, 0x3a, 0xaa, 0x6d, 0xeb } \
    }

class ISwapchain : public ISlangUnknown
{
public:
    struct Desc
    {
        Format format;
        uint32_t width, height;
        uint32_t imageCount;
        ICommandQueue* queue;
        bool enableVSync;
    };
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;

    /// Returns the back buffer image at `index`.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(uint32_t index, ITextureResource** outResource) = 0;

    /// Present the next image in the swapchain.
    virtual SLANG_NO_THROW Result SLANG_MCALL present() = 0;

    /// Returns the index of next back buffer image that will be presented in the next
    /// `present` call. If the swapchain is invalid/out-of-date, this method returns -1.
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() = 0;

    /// Resizes the back buffers of this swapchain. All render target views and framebuffers
    /// referencing the back buffer images must be freed before calling this method.
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) = 0;
};
#define SLANG_UUID_ISwapchain                                                        \
    {                                                                                \
        0xbe91ba6c, 0x784, 0x4308, { 0xa1, 0x0, 0x19, 0xc3, 0x66, 0x83, 0x44, 0xb2 } \
    }

struct DeviceInfo
{
    DeviceType deviceType;

    BindingStyle bindingStyle;

    ProjectionStyle projectionStyle;

    /// An projection matrix that ensures x, y mapping to pixels
    /// is the same on all targets
    float identityProjectionMatrix[16];

    /// The name of the graphics API being used by this device.
    const char* apiName = nullptr;

    /// The name of the graphics adapter.
    const char* adapterName = nullptr;
};

class IDevice: public ISlangUnknown
{
public:
    struct SlangDesc
    {
        slang::IGlobalSession* slangGlobalSession = nullptr; // (optional) A slang global session object. If null will create automatically.

        SlangMatrixLayoutMode defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

        char const* const* searchPaths = nullptr;
        SlangInt            searchPathCount = 0;

        slang::PreprocessorMacroDesc const* preprocessorMacros = nullptr;
        SlangInt                        preprocessorMacroCount = 0;

        const char* targetProfile = nullptr; // (optional) Target shader profile. If null this will be set to platform dependent default.
        SlangFloatingPointMode floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
        SlangOptimizationLevel optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    };

    struct Desc
    {
        DeviceType deviceType = DeviceType::Default;    // The underlying API/Platform of the device.
        const char* adapter = nullptr;                  // Name to identify the adapter to use
        int requiredFeatureCount = 0;                   // Number of required features.
        const char** requiredFeatures = nullptr;        // Array of required feature names, whose size is `requiredFeatureCount`.
        int nvapiExtnSlot = -1;                         // The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
        ISlangFileSystem* shaderCacheFileSystem = nullptr; // The file system for loading cached shader kernels.
        SlangDesc slang = {};                           // Configurations for Slang.
    };

    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) = 0;

        /// Returns a list of features supported by the renderer.
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSlangSession(slang::ISession** outSlangSession) = 0;

    inline ComPtr<slang::ISession> getSlangSession()
    {
        ComPtr<slang::ISession> result;
        getSlangSession(result.writeRef());
        return result;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) = 0;
    inline ComPtr<ITransientResourceHeap> createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc)
    {
        ComPtr<ITransientResourceHeap> result;
        createTransientResourceHeap(desc, result.writeRef());
        return result;
    }

        /// Create a texture resource.
        ///
        /// If `initData` is non-null, then it must point to an array of
        /// `ITextureResource::SubresourceData` with one element for each
        /// subresource of the texture being created.
        ///
        /// The number of subresources in a texture is:
        ///
        ///     effectiveElementCount * mipLevelCount
        ///
        /// where the effective element count is computed as:
        ///
        ///     effectiveElementCount = (isArray ? arrayElementCount : 1) * (isCube ? 6 : 1);
        ///
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    inline SLANG_NO_THROW ComPtr<ITextureResource> createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData = nullptr)
    {
        ComPtr<ITextureResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createTextureResource(initialUsage, desc, initData, resource.writeRef()));
        return resource;
    }

        /// Create a buffer resource
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        IResource::Usage initialUsage,
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) = 0;

    inline SLANG_NO_THROW ComPtr<IBufferResource> createBufferResource(
        IResource::Usage initialUsage,
        const IBufferResource::Desc& desc,
        const void* initData = nullptr)
    {
        ComPtr<IBufferResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createBufferResource(initialUsage, desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) = 0;

    inline ComPtr<ISamplerState> createSamplerState(ISamplerState::Desc const& desc)
    {
        ComPtr<ISamplerState> sampler;
        SLANG_RETURN_NULL_ON_FAIL(createSamplerState(desc, sampler.writeRef()));
        return sampler;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) = 0;

    inline ComPtr<IResourceView> createTextureView(ITextureResource* texture, IResourceView::Desc const& desc)
    {
        ComPtr<IResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createTextureView(texture, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) = 0;

    inline ComPtr<IResourceView> createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc)
    {
        ComPtr<IResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createBufferView(buffer, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebufferLayout(IFramebufferLayout::Desc const& desc, IFramebufferLayout** outFrameBuffer) = 0;
    inline ComPtr<IFramebufferLayout> createFramebufferLayout(IFramebufferLayout::Desc const& desc)
    {
        ComPtr<IFramebufferLayout> fb;
        SLANG_RETURN_NULL_ON_FAIL(createFramebufferLayout(desc, fb.writeRef()));
        return fb;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer) = 0;
    inline ComPtr<IFramebuffer> createFramebuffer(IFramebuffer::Desc const& desc)
    {
        ComPtr<IFramebuffer> fb;
        SLANG_RETURN_NULL_ON_FAIL(createFramebuffer(desc, fb.writeRef()));
        return fb;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) = 0;
    inline ComPtr<IRenderPassLayout> createRenderPassLayout(const IRenderPassLayout::Desc& desc)
    {
        ComPtr<IRenderPassLayout> rs;
        SLANG_RETURN_NULL_ON_FAIL(createRenderPassLayout(desc, rs.writeRef()));
        return rs;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        ISwapchain::Desc const& desc, WindowHandle window, ISwapchain** outSwapchain) = 0;
    inline ComPtr<ISwapchain> createSwapchain(ISwapchain::Desc const& desc, WindowHandle window)
    {
        ComPtr<ISwapchain> swapchain;
        SLANG_RETURN_NULL_ON_FAIL(createSwapchain(desc, window, swapchain.writeRef()));
        return swapchain;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements, UInt inputElementCount, IInputLayout** outLayout) = 0;

    inline ComPtr<IInputLayout> createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount)
    {
        ComPtr<IInputLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createInputLayout(inputElements, inputElementCount, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) = 0;
    inline ComPtr<ICommandQueue> createCommandQueue(const ICommandQueue::Desc& desc)
    {
        ComPtr<ICommandQueue> queue;
        SLANG_RETURN_NULL_ON_FAIL(createCommandQueue(desc, queue.writeRef()));
        return queue;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(slang::TypeReflection* type, IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createShaderObject(slang::TypeReflection* type)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObject(type, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createRootShaderObject(IShaderProgram* program)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createRootShaderObject(program, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) = 0;

    inline ComPtr<IShaderProgram> createProgram(const IShaderProgram::Desc& desc)
    {
        ComPtr<IShaderProgram> program;
        SLANG_RETURN_NULL_ON_FAIL(createProgram(desc, program.writeRef()));
        return program;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc&    desc,
        IPipelineState**                    outState) = 0;

    inline ComPtr<IPipelineState> createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc)
    {
        ComPtr<IPipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createGraphicsPipelineState(desc, state.writeRef()));
        return state;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc&    desc,
        IPipelineState**                     outState) = 0;

    inline ComPtr<IPipelineState> createComputePipelineState(
        const ComputePipelineStateDesc& desc)
    {
        ComPtr<IPipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createComputePipelineState(desc, state.writeRef()));
        return state;
    }

        /// Read back texture resource and stores the result in `outBlob`.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) = 0;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) = 0;

        /// Get the type of this renderer
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const = 0;
};

#define SLANG_UUID_IRenderer                                                             \
    {                                                                                    \
          0x715bdf26, 0x5135, 0x11eb, { 0xAE, 0x93, 0x02, 0x42, 0xAC, 0x13, 0x00, 0x02 } \
    }

// Global public functions

extern "C"
{
    /// Gets the size in bytes of a Format type. Returns 0 if a size is not defined/invalid
    SLANG_GFX_API size_t SLANG_MCALL gfxGetFormatSize(Format format);

    /// Given a type returns a function that can construct it, or nullptr if there isn't one
    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxCreateDevice(const IDevice::Desc* desc, IDevice** outDevice);

    SLANG_GFX_API const char* SLANG_MCALL gfxGetDeviceTypeName(DeviceType type);
}

}// renderer_test
