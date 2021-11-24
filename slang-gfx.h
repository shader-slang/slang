// render.h
#pragma once

#include <float.h>
#include <assert.h>

#include "slang.h"
#include "slang-com-ptr.h"


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

// Needed for building on cygwin with gcc
#undef Always
#undef None

namespace gfx {

using Slang::ComPtr;

typedef SlangResult Result;

// Had to move here, because Options needs types defined here
typedef SlangInt Int;
typedef SlangUInt UInt;
typedef uint64_t DeviceAddress;

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

class ITransientResourceHeap;

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

// Dont' change without keeping in sync with Format
#define GFX_FORMAT(x) \
    x( Unknown, 0, 0) \
    \
    x(R32G32B32A32_TYPELESS, 16, 1) \
    x(R32G32B32_TYPELESS, 12, 1) \
    x(R32G32_TYPELESS, 8, 1) \
    x(R32_TYPELESS, 4, 1) \
    \
    x(R16G16B16A16_TYPELESS, 8, 1) \
    x(R16G16_TYPELESS, 4, 1) \
    x(R16_TYPELESS, 2, 1) \
    \
    x(R8G8B8A8_TYPELESS, 4, 1) \
    x(R8G8_TYPELESS, 2, 1) \
    x(R8_TYPELESS, 1, 1) \
    x(B8G8R8A8_TYPELESS, 4, 1) \
    \
    x(R32G32B32A32_FLOAT, 16, 1) \
    x(R32G32B32_FLOAT, 12, 1) \
    x(R32G32_FLOAT, 8, 1) \
    x(R32_FLOAT, 4, 1) \
    \
    x(R16G16B16A16_FLOAT, 8, 1) \
    x(R16G16_FLOAT, 4, 1) \
    x(R16_FLOAT, 2, 1) \
    \
    x(R32G32B32A32_UINT, 16, 1) \
    x(R32G32B32_UINT, 12, 1) \
    x(R32G32_UINT, 8, 1) \
    x(R32_UINT, 4, 1) \
    \
    x(R16G16B16A16_UINT, 8, 1) \
    x(R16G16_UINT, 4, 1) \
    x(R16_UINT, 2, 1) \
    \
    x(R8G8B8A8_UINT, 4, 1) \
    x(R8G8_UINT, 2, 1) \
    x(R8_UINT, 1, 1) \
    \
    x(R32G32B32A32_SINT, 16, 1) \
    x(R32G32B32_SINT, 12, 1) \
    x(R32G32_SINT, 8, 1) \
    x(R32_SINT, 4, 1) \
    \
    x(R16G16B16A16_SINT, 8, 1) \
    x(R16G16_SINT, 4, 1) \
    x(R16_SINT, 2, 1) \
    \
    x(R8G8B8A8_SINT, 4, 1) \
    x(R8G8_SINT, 2, 1) \
    x(R8_SINT, 1, 1) \
    \
    x(R16G16B16A16_UNORM, 8, 1) \
    x(R16G16_UNORM, 4, 1) \
    x(R16_UNORM, 2, 1) \
    \
    x(R8G8B8A8_UNORM, 4, 1) \
    x(R8G8B8A8_UNORM_SRGB, 4, 1) \
    x(R8G8_UNORM, 2, 1) \
    x(R8_UNORM, 1, 1) \
    x(B8G8R8A8_UNORM, 4, 1) \
    x(B8G8R8A8_UNORM_SRGB, 4, 1) \
    \
    x(R16G16B16A16_SNORM, 8, 1) \
    x(R16G16_SNORM, 4, 1) \
    x(R16_SNORM, 2, 1) \
    \
    x(R8G8B8A8_SNORM, 4, 1) \
    x(R8G8_SNORM, 2, 1) \
    x(R8_SNORM, 1, 1) \
    \
    x(D32_FLOAT, 4, 1) \
    x(D16_UNORM, 2, 1) \
    \
    x(B4G4R4A4_UNORM, 2, 1) \
    x(B5G6R5_UNORM, 2, 1) \
    x(B5G5R5A1_UNORM, 2, 1) \
    \
    x(R9G9B9E5_SHAREDEXP, 4, 1) \
    x(R10G10B10A2_TYPELESS, 4, 1) \
    x(R10G10B10A2_UNORM, 4, 1) \
    x(R10G10B10A2_UINT, 4, 1) \
    x(R11G11B10_FLOAT, 4, 1) \
    \
    x(BC1_UNORM, 8, 16) \
    x(BC1_UNORM_SRGB, 8, 16) \
    x(BC2_UNORM, 16, 16) \
    x(BC2_UNORM_SRGB, 16, 16) \
    x(BC3_UNORM, 16, 16) \
    x(BC3_UNORM_SRGB, 16, 16) \
    x(BC4_UNORM, 8, 16) \
    x(BC4_SNORM, 8, 16) \
    x(BC5_UNORM, 16, 16) \
    x(BC5_SNORM, 16, 16) \
    x(BC6H_UF16, 16, 16) \
    x(BC6H_SF16, 16, 16) \
    x(BC7_UNORM, 16, 16) \
    x(BC7_UNORM_SRGB, 16, 16)

/// Different formats of things like pixels or elements of vertices
/// NOTE! Any change to this type (adding, removing, changing order) - must also be reflected in changes GFX_FORMAT
enum class Format
{
    // D3D formats omitted: 19-22, 44-47, 65-66, 68-70, 73, 76, 79, 82, 88-89, 92-94, 97, 100-114
    // These formats are omitted due to lack of a corresponding Vulkan format. D24_UNORM_S8_UINT (DXGI_FORMAT 45)
    // has a matching Vulkan format but is also omitted as it is only supported by Nvidia.
    Unknown,

    R32G32B32A32_TYPELESS,
    R32G32B32_TYPELESS,
    R32G32_TYPELESS,
    R32_TYPELESS,

    R16G16B16A16_TYPELESS,
    R16G16_TYPELESS,
    R16_TYPELESS,

    R8G8B8A8_TYPELESS,
    R8G8_TYPELESS,
    R8_TYPELESS,
    B8G8R8A8_TYPELESS,

    R32G32B32A32_FLOAT,
    R32G32B32_FLOAT,
    R32G32_FLOAT,
    R32_FLOAT,

    R16G16B16A16_FLOAT,
    R16G16_FLOAT,
    R16_FLOAT,

    R32G32B32A32_UINT,
    R32G32B32_UINT,
    R32G32_UINT,
    R32_UINT,

    R16G16B16A16_UINT,
    R16G16_UINT,
    R16_UINT,

    R8G8B8A8_UINT,
    R8G8_UINT,
    R8_UINT,

    R32G32B32A32_SINT,
    R32G32B32_SINT,
    R32G32_SINT,
    R32_SINT,

    R16G16B16A16_SINT,
    R16G16_SINT,
    R16_SINT,

    R8G8B8A8_SINT,
    R8G8_SINT,
    R8_SINT,

    R16G16B16A16_UNORM,
    R16G16_UNORM,
    R16_UNORM,

    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    R8G8_UNORM,
    R8_UNORM,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,

    R16G16B16A16_SNORM,
    R16G16_SNORM,
    R16_SNORM,

    R8G8B8A8_SNORM,
    R8G8_SNORM,
    R8_SNORM,

    D32_FLOAT,
    D16_UNORM,

    B4G4R4A4_UNORM,
    B5G6R5_UNORM,
    B5G5R5A1_UNORM,

    R9G9B9E5_SHAREDEXP,
    R10G10B10A2_TYPELESS,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R11G11B10_FLOAT,

    BC1_UNORM,
    BC1_UNORM_SRGB,
    BC2_UNORM,
    BC2_UNORM_SRGB,
    BC3_UNORM,
    BC3_UNORM_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UF16,
    BC6H_SF16,
    BC7_UNORM,
    BC7_UNORM_SRGB,

    CountOf,
};

struct FormatInfo
{
    uint8_t channelCount;          ///< The amount of channels in the format. Only set if the channelType is set 
    uint8_t channelType;           ///< One of SlangScalarType None if type isn't made up of elements of type.

    uint32_t blockSizeInBytes;     ///< The size of a block in bytes.
    uint32_t pixelsPerBlock;       ///< The number of pixels contained in a block.
    uint32_t blockWidth;
    uint32_t blockHeight;
};

enum class InputSlotClass
{
    PerVertex, PerInstance
};

struct InputElementDesc
{
    char const* semanticName;
    UInt semanticIndex;
    Format format;
    UInt offset;
    InputSlotClass slotClass;
    UInt bufferSlotIndex;
    UInt instanceDataStepRate;
};

enum class PrimitiveType
{
    Point, Line, Triangle, Patch
};

enum class PrimitiveTopology
{
    TriangleList,
};

enum class ResourceState
{
    Undefined,
    General,
    PreInitialized,
    VertexBuffer,
    IndexBuffer,
    ConstantBuffer,
    StreamOutput,
    ShaderResource,
    UnorderedAccess,
    RenderTarget,
    DepthRead,
    DepthWrite,
    Present,
    IndirectArgument,
    CopySource,
    CopyDestination,
    ResolveSource,
    ResolveDestination,
    AccelerationStructure,
    _Count
};

struct ResourceStateSet
{
public:
    void add(ResourceState state) { m_bitFields |= (1LL << (uint32_t)state); }
    template <typename... TResourceState> void add(ResourceState s, TResourceState... states)
    {
        add(s);
        add(states...);
    }
    bool contains(ResourceState state) const { return (m_bitFields & (1LL << (uint32_t)state)) != 0; }
    ResourceStateSet()
        : m_bitFields(0)
    {}
    ResourceStateSet(const ResourceStateSet& other) = default;
    ResourceStateSet(ResourceState state) { add(state); }
    template <typename... TResourceState> ResourceStateSet(TResourceState... states)
    {
        add(states...);
    }

private:
    uint64_t m_bitFields = 0;
    void add() {}
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

enum class InteropHandleAPI
{
    Unknown,
    D3D12,
    Vulkan,
    CUDA,
    Win32,
    FileDescriptor,
};

struct InteropHandle
{
    InteropHandleAPI api = InteropHandleAPI::Unknown;
    uint64_t handleValue = 0;
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

        /// Base class for Descs
    struct DescBase
    {
        bool hasCpuAccessFlag(AccessFlag::Enum accessFlag) { return (cpuAccessFlags & accessFlag) != 0; }

        Type type = Type::Unknown;
        ResourceState defaultState = ResourceState::Undefined;
        ResourceStateSet allowedStates = ResourceStateSet();
        int cpuAccessFlags = 0;     ///< Combination of Resource::AccessFlag
        InteropHandle existingHandle = {};
        bool isShared = false;
    };

    virtual SLANG_NO_THROW Type SLANG_MCALL getType() = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeResourceHandle(InteropHandle* outHandle) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) = 0;
	
    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) = 0;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getDebugName() = 0;

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
        size_t sizeInBytes = 0;     ///< Total size in bytes
        int elementSize = 0;        ///< Get the element stride. If > 0, this is a structured buffer
        Format format = Format::Unknown;
    };

    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() = 0;
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() = 0;
};
#define SLANG_UUID_IBufferResource                                                     \
    {                                                                                  \
        0x1b274efe, 0x5e37, 0x492b, { 0x82, 0x6e, 0x7e, 0xe7, 0xe8, 0xf5, 0xa4, 0x9b } \
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

struct BufferRange
{
    uint32_t firstElement;
    uint32_t elementCount;
};

enum class TextureAspect : uint32_t
{
    Default = 0,
    Color = 0x00000001,
    Depth = 0x00000002,
    Stencil = 0x00000004,
    MetaData = 0x00000008,
    Plane0 = 0x00000010,
    Plane1 = 0x00000020,
    Plane2 = 0x00000040,
};

struct SubresourceRange
{
    TextureAspect aspectMask;
    uint32_t mipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer; // For Texture3D, this is WSlice.
    uint32_t layerCount; // For cube maps, this is a multiple of 6.
};

class ITextureResource: public IResource
{
public:
    static const uint32_t kTexturePitchAlignment = 256;

    struct Offset3D
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        Offset3D() = default;
        Offset3D(uint32_t _x, uint32_t _y, uint32_t _z) :x(_x), y(_y), z(_z) {}
    };

    struct SampleDesc
    {
        int numSamples = 1;                     ///< Number of samples per pixel
        int quality = 0;                        ///< The quality measure for the samples
    };

    struct Size
    {
        int width = 0;              ///< Width in pixels
        int height = 0;             ///< Height in pixels (if 2d or 3d)
        int depth = 0;              ///< Depth (if 3d)
    };

    struct Desc: public DescBase
    {
        Size size;

        int arraySize = 0;          ///< Array size

        int numMipLevels = 0;       ///< Number of mip levels - if 0 will create all mip levels
        Format format;              ///< The resources format
        SampleDesc sampleDesc;      ///< How the resource is sampled
        ClearValue optimalClearValue;
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
        AccelerationStructure,
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
        SubresourceRange subresourceRange;
        BufferRange bufferRange;
    };
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() = 0;
};
#define SLANG_UUID_IResourceView                                                      \
    {                                                                                 \
        0x7b6c4926, 0x884, 0x408c, { 0xad, 0x8a, 0x50, 0x3a, 0x8e, 0x23, 0x98, 0xa4 } \
    }

class IAccelerationStructure : public IResourceView
{
public:
    enum class Kind
    {
        TopLevel,
        BottomLevel
    };

    struct BuildFlags
    {
        // The enum values are intentionally consistent with
        // D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS.
        enum Enum
        {
            None,
            AllowUpdate = 1,
            AllowCompaction = 2,
            PreferFastTrace = 4,
            PreferFastBuild = 8,
            MinimizeMemory = 16,
            PerformUpdate = 32
        };
    };

    enum class GeometryType
    {
        Triangles, ProcedurePrimitives
    };

    struct GeometryFlags
    {
        // The enum values are intentionally consistent with
        // D3D12_RAYTRACING_GEOMETRY_FLAGS.
        enum Enum
        {
            None,
            Opaque = 1,
            NoDuplicateAnyHitInvocation = 2
        };
    };

    struct TriangleDesc
    {
        DeviceAddress transform3x4;
        Format indexFormat;
        Format vertexFormat;
        uint32_t indexCount;
        uint32_t vertexCount;
        DeviceAddress indexData;
        DeviceAddress vertexData;
        uint64_t vertexStride;
    };

    struct ProceduralAABB
    {
        float minX;
        float minY;
        float minZ;
        float maxX;
        float maxY;
        float maxZ;
    };

    struct ProceduralAABBDesc
    {
        /// Number of AABBs.
        uint64_t count;

        /// Pointer to an array of `ProceduralAABB` values in device memory.
        DeviceAddress data;

        /// Stride in bytes of the AABB values array.
        uint64_t stride;
    };

    struct GeometryDesc
    {
        GeometryType type;
        GeometryFlags::Enum flags;
        union
        {
            TriangleDesc triangles;
            ProceduralAABBDesc proceduralAABBs;
        } content;
    };

    struct GeometryInstanceFlags
    {
        // The enum values are kept consistent with D3D12_RAYTRACING_INSTANCE_FLAGS
        // and VkGeometryInstanceFlagBitsKHR.
        enum Enum : uint32_t
        {
            None = 0,
            TriangleFacingCullDisable = 0x00000001,
            TriangleFrontCounterClockwise = 0x00000002,
            ForceOpaque = 0x00000004,
            NoOpaque = 0x00000008
        };
    };

    // The layout of this struct is intentionally consistent with D3D12_RAYTRACING_INSTANCE_DESC
    // and VkAccelerationStructureInstanceKHR.
    struct InstanceDesc
    {
        float transform[3][4];
        uint32_t instanceID : 24;
        uint32_t instanceMask : 8;
        uint32_t instanceContributionToHitGroupIndex : 24;
        GeometryInstanceFlags::Enum flags : 8;
        DeviceAddress accelerationStructure;
    };

    struct PrebuildInfo
    {
        uint64_t resultDataMaxSize;
        uint64_t scratchDataSize;
        uint64_t updateScratchDataSize;
    };

    struct BuildInputs
    {
        Kind kind;

        BuildFlags::Enum flags;

        int32_t descCount;

        /// Array of `InstanceDesc` values in device memory.
        /// Used when `kind` is `TopLevel`.
        DeviceAddress instanceDescs;

        /// Array of `GeometryDesc` values.
        /// Used when `kind` is `BottomLevel`.
        const GeometryDesc* geometryDescs;
    };

    struct CreateDesc
    {
        Kind kind;
        IBufferResource* buffer;
        uint64_t offset;
        uint64_t size;
    };

    struct BuildDesc
    {
        BuildInputs inputs;
        IAccelerationStructure* source;
        IAccelerationStructure* dest;
        DeviceAddress scratchData;
    };

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() = 0;
};
#define SLANG_UUID_IAccelerationStructure                                             \
    {                                                                                 \
        0xa5cdda3c, 0x1d4e, 0x4df7, { 0x8e, 0xf2, 0xb7, 0x3f, 0xce, 0x4, 0xde, 0x3b } \
    }

class IFence : public ISlangUnknown
{
public:
    struct Desc
    {
        uint64_t initialValue = 0;
    };

    /// Returns the currently signaled value on the device.
    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) = 0;

    /// Signals the fence from the host with the specified value.
    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) = 0;
};
#define SLANG_UUID_IFence                                                             \
    {                                                                                 \
        0x7fe1c283, 0xd3f4, 0x48ed, { 0xaa, 0xf3, 0x1, 0x51, 0x96, 0x4e, 0x7c, 0xb5 } \
    }

struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
    uint32_t getHashCode() const
    {
        return (uint32_t)(((bindingRangeIndex << 20) + bindingArrayIndex) ^ uniformOffset);
    }
    bool operator==(const ShaderOffset& other) const
    {
        return uniformOffset == other.uniformOffset
            && bindingRangeIndex == other.bindingRangeIndex
            && bindingArrayIndex == other.bindingArrayIndex;
    }
    bool operator!=(const ShaderOffset& other) const
    {
        return !this->operator==(other);
    }
    bool operator<(const ShaderOffset& other) const
    {
        if (bindingRangeIndex < other.bindingRangeIndex)
            return true;
        if (bindingRangeIndex > other.bindingRangeIndex)
            return false;
        if (bindingArrayIndex < other.bindingArrayIndex)
            return true;
        if (bindingArrayIndex > other.bindingArrayIndex)
            return false;
        return uniformOffset < other.uniformOffset;
    }
    bool operator<=(const ShaderOffset& other) const { return (*this == other) || (*this) < other; }
    bool operator>(const ShaderOffset& other) const { return other < *this; }
    bool operator>=(const ShaderOffset& other) const { return other <= *this; }
};

enum class ShaderObjectContainerType
{
    None, Array, StructuredBuffer
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
    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() = 0;
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

        /// Manually overrides the specialization argument for the sub-object binding at `offset`.
        /// Specialization arguments are passed to the shader compiler to specialize the type
        /// of interface-typed shader parameters.
    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(
        ShaderOffset const& offset,
        const slang::SpecializationArg* args,
        uint32_t count) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentVersion(
        ITransientResourceHeap* transientHeap,
        IShaderObject** outObject) = 0;

        /// Copies contents from another shader object to this object.
    virtual SLANG_NO_THROW Result SLANG_MCALL copyFrom(IShaderObject* other) = 0;

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() = 0;

    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() = 0;

        /// Use the provided constant buffer instead of the internally created one.
    virtual SLANG_NO_THROW Result SLANG_MCALL setConstantBufferOverride(IBufferResource* constantBuffer) = 0;
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
    FillMode fillMode = FillMode::Solid;
    CullMode cullMode = CullMode::None;
    FrontFaceMode frontFace = FrontFaceMode::CounterClockwise;
    int32_t depthBias = 0;
    float depthBiasClamp = 0.0f;
    float slopeScaledDepthBias = 0.0f;
    bool depthClipEnable = true;
    bool scissorEnable = false;
    bool multisampleEnable = false;
    bool antialiasedLineEnable = false;
    bool enableConservativeRasterization = false;
    uint32_t forcedSampleCount = 0;
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
    bool enableBlend = false;
    LogicOp                 logicOp     = LogicOp::NoOp;
    RenderTargetWriteMaskT  writeMask   = RenderTargetWriteMask::EnableAll;
};

struct BlendDesc
{
    TargetBlendDesc const*  targets     = nullptr;
    UInt                    targetCount = 0;

    bool alphaToCoverageEnable  = false;
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

struct RayTracingPipelineFlags
{
    enum Enum : uint32_t
    {
        None = 0,
        SkipTriangles = 1,
        SkipProcedurals = 2,
    };
};

struct HitGroupDesc
{
    const char* closestHitEntryPoint = nullptr;
    const char* anyHitEntryPoint = nullptr;
    const char* intersectionEntryPoint = nullptr;
};

struct RayTracingPipelineStateDesc
{
    IShaderProgram* program = nullptr;
    int32_t hitGroupCount;
    const HitGroupDesc* hitGroups;
    int32_t shaderTableHitGroupCount;
    int32_t* shaderTableHitGroupIndices;
    int maxRecursion;
    int maxRayPayloadSize;
    RayTracingPipelineFlags::Enum flags;
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
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
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

enum class QueryType
{
    Timestamp,
    AccelerationStructureCompactedSize,
    AccelerationStructureSerializedSize,
    AccelerationStructureCurrentSize,
};

class IQueryPool : public ISlangUnknown
{
public:
    struct Desc
    {
        QueryType type;
        SlangInt count;
    };
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) = 0;
};
#define SLANG_UUID_IQueryPool                                                         \
    { 0xc2cc3784, 0x12da, 0x480a, { 0xa8, 0x74, 0x8b, 0x31, 0x96, 0x1c, 0xa4, 0x36 } }


class ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* queryPool, SlangInt queryIndex) = 0;
};

struct IndirectDispatchArguments
{
    uint32_t ThreadGroupCountX;
    uint32_t ThreadGroupCountY;
    uint32_t ThreadGroupCountZ;
};

struct IndirectDrawArguments
{
    uint32_t VertexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartVertexLocation;
    uint32_t StartInstanceLocation;
};

struct IndirectDrawIndexedArguments
{
    uint32_t IndexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartIndexLocation;
    int32_t  BaseVertexLocation;
    uint32_t StartInstanceLocation;
};

struct SamplePosition
{
    int8_t x;
    int8_t y;
};

struct ClearResourceViewFlags
{
    enum Enum : uint32_t
    {
        None = 0,
        ClearDepth = 1,
        ClearStencil = 2,
        FloatClearValues = 4
    };
};

class IRenderCommandEncoder : public ICommandEncoder
{
public:
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRooShaderObject` stays alive during
    // the execution of the command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) = 0;
    inline IShaderObject* bindPipeline(IPipelineState* state)
    {
        IShaderObject* rootObject = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(bindPipeline(state, &rootObject));
        return rootObject;
    }

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
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer,
        uint64_t countOffset) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer,
        uint64_t countOffset) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL setSamplePositions(
        uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition* samplePositions) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawInstanced(
        UInt vertexCount,
        UInt instanceCount,
        UInt startVertex,
        UInt startInstanceLocation) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedInstanced(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndexLocation,
        int32_t baseVertexLocation,
        uint32_t startInstanceLocation) = 0;
};

class IComputeCommandEncoder : public ICommandEncoder
{
public:
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRooShaderObject` stays alive during
    // the execution of the command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) = 0;
    inline IShaderObject* bindPipeline(IPipelineState* state)
    {
        IShaderObject* rootObject = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(bindPipeline(state, &rootObject));
        return rootObject;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchComputeIndirect(IBufferResource* cmdBuffer, uint64_t offset) = 0;
};

class IResourceCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) = 0;

    /// Copies texture to a buffer. Each row is aligned to kTexturePitchAlignment.
    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        size_t dstSize,
        ITextureResource* src,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Offset3D extent,
        ITextureResource::SubresourceData* subResourceData,
        size_t subResourceDataCount) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        size_t count,
        ITextureResource* const* textures,
        ResourceState src,
        ResourceState dst) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
        ITextureResource* texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        size_t count,
        IBufferResource* const* buffers,
        ResourceState src,
        ResourceState dst) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
        ITextureResource* source,
        SubresourceRange sourceRange,
        ITextureResource* dest,
        SubresourceRange destRange) = 0;
};

enum class AccelerationStructureCopyMode
{
    Clone, Compact
};

struct AccelerationStructureQueryDesc
{
    QueryType queryType;

    IQueryPool* queryPool;

    int32_t firstQueryIndex;
};

class IRayTracingCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
        const IAccelerationStructure::BuildDesc& desc,
        int propertyQueryCount,
        AccelerationStructureQueryDesc* queryDescs) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
        IAccelerationStructure* dest,
        IAccelerationStructure* src,
        AccelerationStructureCopyMode mode) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
        int accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        int queryCount,
        AccelerationStructureQueryDesc* queryDescs) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure* source) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        deserializeAccelerationStructure(IAccelerationStructure* dest, DeviceAddress source) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL memoryBarrier(
        int count,
        IAccelerationStructure* const* structures,
        AccessFlag::Enum sourceAccess,
        AccessFlag::Enum destAccess) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) = 0;

    /// Issues a dispatch command to start ray tracing workload with a ray tracing pipeline.
    /// `rayGenShaderName` specifies the name of the ray generation shader to launch. Pass nullptr for
    /// the first ray generation shader defined in `raytracingPipeline`.
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
        const char* rayGenShaderName,
        int32_t width,
        int32_t height,
        int32_t depth) = 0;
};
#define SLANG_UUID_IRayTracingCommandEncoder                                           \
    {                                                                                  \
        0x9a672b87, 0x5035, 0x45e3, { 0x96, 0x7c, 0x1f, 0x85, 0xcd, 0xb3, 0x63, 0x4f } \
    }

class ICommandBuffer : public ISlangUnknown
{
public:
    // For D3D12, this is the pointer to the buffer. For Vulkan, this is the buffer itself.
    typedef uint64_t NativeHandle;

    // Only one encoder may be open at a time. User must call `ICommandEncoder::endEncoding`
    // before calling other `encode*Commands` methods.
    // Once `endEncoding` is called, the `ICommandEncoder` object becomes obsolete and is
    // invalid for further use. To continue recording, the user must request a new encoder
    // object by calling one of the `encode*Commands` methods again.
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) = 0;
    IRenderCommandEncoder*
        encodeRenderCommands(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
    {
        IRenderCommandEncoder* result;
        encodeRenderCommands(renderPass, framebuffer, &result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) = 0;
    IComputeCommandEncoder* encodeComputeCommands()
    {
        IComputeCommandEncoder* result;
        encodeComputeCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) = 0;
    IResourceCommandEncoder* encodeResourceCommands()
    {
        IResourceCommandEncoder* result;
        encodeResourceCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) = 0;
    IRayTracingCommandEncoder* encodeRayTracingCommands()
    {
        IRayTracingCommandEncoder* result;
        encodeRayTracingCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL close() = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) = 0;
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

    // For D3D12, this is the pointer to the queue. For Vulkan, this is the queue itself.
    typedef uint64_t NativeHandle;

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        uint32_t count,
        ICommandBuffer* const* commandBuffers,
        IFence* fenceToSignal,
        uint64_t newFenceValue) = 0;
    inline void executeCommandBuffer(
        ICommandBuffer* commandBuffer, IFence* fenceToSignal = nullptr, uint64_t newFenceValue = 0)
    {
        executeCommandBuffers(1, &commandBuffer, fenceToSignal, newFenceValue);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL wait() = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(NativeHandle* outHandle) = 0;
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

    /// The clock frequency used in timestamp queries.
    uint64_t timestampFrequency = 0;
};

enum class DebugMessageType
{
    Info, Warning, Error
};
enum class DebugMessageSource
{
    Layer, Driver, Slang
};
class IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
        handleMessage(DebugMessageType type, DebugMessageSource source, const char* message) = 0;
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
        SlangTargetFlags targetFlags = 0;
        SlangLineDirectiveMode lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
    };

    struct InteropHandles
    {
        InteropHandle handles[3] = {};
    };

    struct Desc
    {
        // The underlying API/Platform of the device.
        DeviceType deviceType = DeviceType::Default;
        // The device's handles (if they exist) and their associated API. For D3D12, this contains a single InteropHandle
        // for the ID3D12Device. For Vulkan, the first InteropHandle is the VkInstance, the second is the VkPhysicalDevice,
        // and the third is the VkDevice. For CUDA, this only contains a single value for the CUDADevice.
        InteropHandles existingDeviceHandles;
        // Name to identify the adapter to use
        const char* adapter = nullptr;
        // Number of required features.
        int requiredFeatureCount = 0;
        // Array of required feature names, whose size is `requiredFeatureCount`.
        const char** requiredFeatures = nullptr;
        // The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
        int nvapiExtnSlot = -1;
        // The file system for loading cached shader kernels. The layer does not maintain a strong reference to the object,
        // instead the user is responsible for holding the object alive during the lifetime of an `IDevice`.
        ISlangFileSystem* shaderCacheFileSystem = nullptr;
        // Configurations for Slang compiler.
        SlangDesc slang = {};
    };

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeDeviceHandles(InteropHandles* outHandles) = 0;

    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) = 0;

        /// Returns a list of features supported by the renderer.
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates) = 0;

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
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    inline SLANG_NO_THROW ComPtr<ITextureResource> createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData = nullptr)
    {
        ComPtr<ITextureResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createTextureResource(desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromNativeHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        ITextureResource** outResource) = 0;

        /// Create a buffer resource
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) = 0;

    inline SLANG_NO_THROW ComPtr<IBufferResource> createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData = nullptr)
    {
        ComPtr<IBufferResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createBufferResource(desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromNativeHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromSharedHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) = 0;

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

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createShaderObject(slang::TypeReflection* type)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObject(type, ShaderObjectContainerType::None, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableRootShaderObject(
        IShaderProgram* program,
        IShaderObject** outObject) = 0;

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

    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc, IPipelineState** outState) = 0;

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

    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc, IQueryPool** outPool) = 0;

    
    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFence(const IFence::Desc& desc, IFence** outFence) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFences(
        uint32_t fenceCount,
        IFence** fences,
        uint64_t* values,
        bool waitForAll,
        uint64_t timeout) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureAllocationInfo(
        const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment) = 0;
};

#define SLANG_UUID_IDevice                                                               \
    {                                                                                    \
          0x715bdf26, 0x5135, 0x11eb, { 0xAE, 0x93, 0x02, 0x42, 0xAC, 0x13, 0x00, 0x02 } \
    }

// Global public functions

extern "C"
{
    /// Checks if format is compressed
    SLANG_GFX_API bool gfxIsCompressedFormat(Format format);

    /// Checks if format is typeless
    SLANG_GFX_API bool gfxIsTypelessFormat(Format format);

    /// Gets information about the format 
    SLANG_GFX_API SlangResult gfxGetFormatInfo(Format format, FormatInfo* outInfo);

    /// Given a type returns a function that can construct it, or nullptr if there isn't one
    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxCreateDevice(const IDevice::Desc* desc, IDevice** outDevice);

    /// Sets a callback for receiving debug messages.
    /// The layer does not hold a strong reference to the callback object.
    /// The user is responsible for holding the callback object alive.
    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxSetDebugCallback(IDebugCallback* callback);

    /// Enables debug layer. The debug layer will check all `gfx` calls and verify that uses are valid.
    SLANG_GFX_API void SLANG_MCALL gfxEnableDebugLayer();

    SLANG_GFX_API const char* SLANG_MCALL gfxGetDeviceTypeName(DeviceType type);
}

}// renderer_test
