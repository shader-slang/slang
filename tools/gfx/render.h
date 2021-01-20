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

class IShaderProgram: public ISlangUnknown
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
            return BindFlag::UnorderedAccess;
        case Usage::PixelShaderResource:
            return BindFlag::PixelShaderResource;
        case Usage::NonPixelShaderResource:
            return BindFlag::NonPixelShaderResource;
        case Usage::GenericRead:
            return BindFlag::Enum(
                BindFlag::PixelShaderResource |
                BindFlag::NonPixelShaderResource);
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
                {
                    return numMipMaps * arrSize;
                }
            case IResource::Type::Texture3D:
                {
                    // can't have arrays of 3d textures
                    assert(this->arraySize <= 1);
                    return numMipMaps * this->size.depth;
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
        void setDefaults(Usage initialUsage)
        {
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

class IDescriptorSetLayout : public ISlangUnknown
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
#define SLANG_UUID_IDescriptorSetLayout                                                 \
    {                                                                                  \
        0x9fe39a2f, 0xdf8b, 0x4690, { 0x90, 0x6a, 0x10, 0x1e, 0xed, 0xf9, 0xbe, 0xc0 } \
    }


class IPipelineLayout : public ISlangUnknown
{
public:
    struct DescriptorSetDesc
    {
        IDescriptorSetLayout*    layout          = nullptr;

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
            IDescriptorSetLayout*    layout)
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
#define SLANG_UUID_IPipelineLayout                                                      \
    {                                                                                  \
        0x9d644a9a, 0x3e6f, 0x4350, { 0xa3, 0x5a, 0xe8, 0xe3, 0xbc, 0xef, 0xb9, 0xcf } \
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

    struct Desc
    {
        Type    type;
        Format  format;
    };
};
#define SLANG_UUID_IResourceView                                                       \
    {                                                                                 \
        0x7b6c4926, 0x884, 0x408c, { 0xad, 0x8a, 0x50, 0x3a, 0x8e, 0x23, 0x98, 0xa4 } \
    }


class IDescriptorSet : public ISlangUnknown
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL setConstantBuffer(UInt range, UInt index, IBufferResource* buffer) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setResource(UInt range, UInt index, IResourceView* view) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setSampler(UInt range, UInt index, ISamplerState* sampler) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setCombinedTextureSampler(
        UInt range,
        UInt index,
        IResourceView*   textureView,
        ISamplerState*   sampler) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setRootConstants(
        UInt range,
        UInt offset,
        UInt size,
        void const* data) = 0;
};
#define SLANG_UUID_IDescriptorSet                                                     \
    {                                                                                \
        0x29a881ea, 0xd7, 0x41d4, { 0xa3, 0x2d, 0x6c, 0x78, 0x4b, 0x79, 0xda, 0x2e } \
    }


struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
};

class IShaderObjectLayout : public ISlangUnknown
{};
#define SLANG_UUID_IShaderObjectLayout                                                 \
    {                                                                                 \
        0x27f3f67e, 0xa49d, 0x4aae, { 0xa6, 0xd, 0xfa, 0xc2, 0x6b, 0x1c, 0x10, 0x7c } \
    }


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
    IShaderProgram*      program;
    // Application should set either pipelineLayout or rootShaderObjectLayout, but not both.
    IPipelineLayout* pipelineLayout = nullptr;
    // Application should set either pipelineLayout or rootShaderObjectLayout, but not both.
    IShaderObjectLayout* rootShaderObjectLayout = nullptr;
    IInputLayout*        inputLayout;
    UInt                framebufferWidth;
    UInt                framebufferHeight;
    UInt                renderTargetCount = 0; // Not used if rootShaderObjectLayout is set.
    DepthStencilDesc    depthStencil;
    RasterizerDesc      rasterizer;
    BlendDesc           blend;
};

struct ComputePipelineStateDesc
{
    IShaderProgram*  program;
    IPipelineLayout* pipelineLayout = nullptr;
    IShaderObjectLayout* rootShaderObjectLayout = nullptr;
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

class IRenderer: public ISlangUnknown
{
public:

    struct Desc
    {
        int width = 0;                                  ///< Width in pixels
        int height = 0;                                 ///< height in pixels
        const char* adapter = nullptr;                  ///< Name to identify the adapter to use
        int requiredFeatureCount = 0;                   ///< Number of required features.
        const char** requiredFeatures = nullptr;        ///< Array of required feature names, whose size is `requiredFeatureCount`.
        int nvapiExtnSlot = -1;                         ///< The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
    };

        // Will return with SLANG_E_NOT_AVAILABLE if NVAPI can't be initialized and nvapiExtnSlot >= 0
    virtual SLANG_NO_THROW Result SLANG_MCALL initialize(const Desc& desc, void* inWindowHandle) = 0;

    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) = 0;

        /// Returns a list of features supported by the renderer.
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL presentFrame() = 0;

    virtual SLANG_NO_THROW ITextureResource::Desc SLANG_MCALL getSwapChainTextureDesc() = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::Data* initData,
        ITextureResource** outResource) = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    inline SLANG_NO_THROW ComPtr<ITextureResource> createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::Data* initData = nullptr)
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

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements, UInt inputElementCount, IInputLayout** outLayout) = 0;

    inline ComPtr<IInputLayout> createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount)
    {
        ComPtr<IInputLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createInputLayout(inputElements, inputElementCount, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) = 0;

    inline ComPtr<IDescriptorSetLayout> createDescriptorSetLayout(const IDescriptorSetLayout::Desc& desc)
    {
        ComPtr<IDescriptorSetLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createDescriptorSetLayout(desc, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObjectLayout** outLayout) = 0;

    inline ComPtr<IShaderObjectLayout> createShaderObjectLayout(slang::TypeLayoutReflection* typeLayout)
    {
        ComPtr<IShaderObjectLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObjectLayout(typeLayout, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObjectLayout(
        slang::ProgramLayout* layout, IShaderObjectLayout** outLayout) = 0;

    inline ComPtr<IShaderObjectLayout> createRootShaderObjectLayout(slang::ProgramLayout* layout)
    {
        ComPtr<IShaderObjectLayout> result;
        SLANG_RETURN_NULL_ON_FAIL(createRootShaderObjectLayout(layout, result.writeRef()));
        return result;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(IShaderObjectLayout* layout, IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createShaderObject(IShaderObjectLayout* layout)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObject(layout, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRootShaderObject(IShaderObjectLayout* layout, IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createRootShaderObject(IShaderObjectLayout* layout)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createRootShaderObject(layout, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL bindRootShaderObject(PipelineType pipelineType, IShaderObject* object) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) = 0;

    inline ComPtr<IPipelineLayout> createPipelineLayout(const IPipelineLayout::Desc& desc)
    {
        ComPtr<IPipelineLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createPipelineLayout(desc, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet** outDescriptorSet) = 0;

    inline ComPtr<IDescriptorSet> createDescriptorSet(IDescriptorSetLayout* layout)
    {
        ComPtr<IDescriptorSet> descriptorSet;
        SLANG_RETURN_NULL_ON_FAIL(createDescriptorSet(layout, descriptorSet.writeRef()));
        return descriptorSet;
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

        /// Captures the back buffer and stores the result in surfaceOut. If the surface contains data - it will either be overwritten (if same size and format), or freed and a re-allocated.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL captureScreenSurface(void* buffer, size_t *inOutBufferSize, size_t* outRowPitch, size_t* outPixelSize) = 0;

    virtual SLANG_NO_THROW void* SLANG_MCALL map(IBufferResource* buffer, MapFlavor flavor) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL unmap(IBufferResource* buffer) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) = 0;
    inline void setVertexBuffer(UInt slot, IBufferResource* buffer, UInt stride, UInt offset = 0);

    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset = 0) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL setDepthStencilTarget(IResourceView* depthStencilView) = 0;

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

    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(PipelineType pipelineType, IPipelineState* state) = 0;

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
inline void IRenderer::setVertexBuffer(UInt slot, IBufferResource* buffer, UInt stride, UInt offset)
{
    setVertexBuffers(slot, 1, &buffer, &stride, &offset);
}

// Global public functions

extern "C"
{
    typedef SlangResult(SLANG_MCALL * SGRendererCreateFunc)(IRenderer** outRenderer);

    /// Gets the size in bytes of a Format type. Returns 0 if a size is not defined/invalid
    SLANG_GFX_API size_t SLANG_MCALL gfxGetFormatSize(Format format);

    /// Gets the binding style from the type
    SLANG_GFX_API BindingStyle SLANG_MCALL gfxGetBindingStyle(RendererType type);

    /// Given a renderer type, gets a projection style
    SLANG_GFX_API ProjectionStyle SLANG_MCALL gfxGetProjectionStyle(RendererType type);

    /// Given the projection style returns an 'identity' matrix, which ensures x,y mapping to pixels
    /// is the same on all targets
    SLANG_GFX_API void SLANG_MCALL
        gfxGetIdentityProjection(ProjectionStyle style, float projMatrix[16]);

    /// Get the name of the renderer
    SLANG_GFX_API const char* SLANG_MCALL gfxGetRendererName(RendererType type);

    /// Given a type returns a function that can construct it, or nullptr if there isn't one
    SLANG_GFX_API SGRendererCreateFunc SLANG_MCALL gfxGetCreateFunc(RendererType type);
}

}// renderer_test
