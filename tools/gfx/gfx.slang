import slang;

namespace gfx
{
typedef slang.Result Result;

typedef intptr_t Int;
typedef uintptr_t UInt;
typedef uint64_t DeviceAddress;
typedef int GfxIndex;
typedef int GfxCount;
typedef intptr_t Size;
typedef intptr_t Offset;

const uint64_t kTimeoutInfinite = 0xFFFFFFFFFFFFFFFF;

enum class StructType
{
    D3D12ExtendedDesc,
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

enum class AccessFlag
{
    None,
    Read,
    Write,
};

static const GfxCount kMaxRenderTargetCount = 8;

// Defines how linking should be performed for a shader program.
enum class LinkingStyle
{
    // Compose all entry-points in a single program, then compile all entry-points together with the same
    // set of root shader arguments.
    SingleProgram,

    // Link and compile each entry-point individually, potentially with different specializations.
    SeparateEntryPointCompilation
};

enum class ShaderModuleSourceType
{
    SlangSource,           // a slang source string in memory.
    SlangModuleBinary,     // a slang module binary code in memory.
    SlangSourceFile,       // a slang source from file.
    SlangModuleBinaryFile, // a slang module binary code from file.
};

struct ShaderProgramDesc2
{
    ShaderModuleSourceType sourceType;
    void *sourceData;
    Size sourceDataSize;

    // Number of entry points to include in the shader program. 0 means include all entry points
    // defined in the module.
    GfxCount entryPointCount = 0;
    // Names of entry points to include in the shader program. The size of the array must be
    // `entryPointCount`.
    NativeString* entryPointNames = nullptr;
};

[COM("9d32d0ad-915c-4ffd-91e2-508554a04a76")]
interface IShaderProgram
{
    slang::TypeReflection* findTypeByName(NativeString name);
};

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
    B8G8R8X8_UNORM,
    B8G8R8X8_UNORM_SRGB,

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

    _Count,
};

struct FormatInfo
{
    GfxCount channelCount; ///< The amount of channels in the format. Only set if the channelType is set
    uint8_t channelType;   ///< One of SlangScalarType None if type isn't made up of elements of type. TODO: Change to uint32_t?

    Size blockSizeInBytes;   ///< The size of a block in bytes.
    GfxCount pixelsPerBlock; ///< The number of pixels contained in a block.
    GfxCount blockWidth;     ///< The width of a block in pixels.
    GfxCount blockHeight;    ///< The height of a block in pixels.
};

enum class InputSlotClass
{
    PerVertex, PerInstance
};

struct InputElementDesc
{
    NativeString semanticName; ///< The name of the corresponding parameter in shader code.
    GfxIndex semanticIndex;   ///< The index of the corresponding parameter in shader code. Only needed if multiple parameters share a semantic name.
    Format format;            ///< The format of the data being fetched for this element.
    Offset offset;            ///< The offset in bytes of this element from the start of the corresponding chunk of vertex stream data.
    GfxIndex bufferSlotIndex; ///< The index of the vertex stream to fetch this element's data from.
};

struct VertexStreamDesc
{
    Size stride;                   ///< The stride in bytes for this vertex stream.
    InputSlotClass slotClass;      ///< Whether the stream contains per-vertex or per-instance data.
    GfxCount instanceDataStepRate; ///< How many instances to draw per chunk of data.
};

enum class PrimitiveType
{
    Point, Line, Triangle, Patch
};

enum class PrimitiveTopology
{
    TriangleList, TriangleStrip, PointList, LineList, LineStrip
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
    AccelerationStructureBuildInput,
    _Count
};

struct ResourceStateSet
{
    uint64_t m_bitFields;

    [mutating]
    void add(ResourceState state) { m_bitFields |= (1LL << (uint32_t)state); }

    bool contains(ResourceState state) { return (m_bitFields & (1LL << (uint32_t)state)) != 0; }
    __init() { m_bitFields = 0; }
    __init(ResourceState state) { add(state); }
};

ResourceStateSet operator &(ResourceStateSet val, ResourceStateSet that)
{
    ResourceStateSet result;
    result.m_bitFields = val.m_bitFields & that.m_bitFields;
    return result;
}

/// Describes how memory for the resource should be allocated for CPU access.
enum class MemoryType
{
    DeviceLocal,
    Upload,
    ReadBack,
};

enum class InteropHandleAPI
{
    Unknown,
    D3D12,                    // A D3D12 object pointer.
    Vulkan,                   // A general Vulkan object handle.
    CUDA,                     // A general CUDA object handle.
    Win32,                    // A general Win32 HANDLE.
    FileDescriptor,           // A file descriptor.
    DeviceAddress,            // A device address.
    D3D12CpuDescriptorHandle, // A D3D12_CPU_DESCRIPTOR_HANDLE value.
};

struct InteropHandle
{
    InteropHandleAPI api = InteropHandleAPI::Unknown;
    uint64_t handleValue;
};

// Declare opaque type
struct InputLayoutDesc
{
    InputElementDesc *inputElements;
    GfxCount inputElementCount;
    VertexStreamDesc *vertexStreams;
    GfxCount vertexStreamCount;
};

[COM("45223711-a84b-455c-befa-4937421e8e2e")]
interface IInputLayout
{   
};

/// The type of resource.
/// NOTE! The order needs to be such that all texture types are at or after Texture1D (otherwise isTexture won't work correctly)
enum class ResourceType
{
    Unknown,     ///< Unknown
    Buffer,      ///< A buffer (like a constant/index/vertex buffer)
    Texture1D,   ///< A 1d texture
    Texture2D,   ///< A 2d texture
    Texture3D,   ///< A 3d texture
    TextureCube, ///< A cubemap consists of 6 Texture2D like faces
    _Count,
};

/// Base class for Descs
struct ResourceDescBase
{
    ResourceType type;
    ResourceState defaultState;
    ResourceStateSet allowedStates;
    MemoryType memoryType;
    InteropHandle existingHandle;
    bool isShared;
};

[COM("a0e39f34-8398-4522-95c2-ebc0f984ef3f")]
interface IResource
{
    ResourceType getType();
    Result getNativeResourceHandle(out InteropHandle outHandle);
    Result getSharedHandle(out InteropHandle outHandle);
    Result setDebugName(NativeString name);
    NativeString getDebugName();
};

struct MemoryRange
{
    // TODO: Change to Offset/Size?
    uint64_t offset;
    uint64_t size;
};

struct BufferResourceDesc : ResourceDescBase
{
    Size sizeInBytes = 0;        ///< Total size in bytes
    Size elementSize = 0;        ///< Get the element stride. If > 0, this is a structured buffer
    Format format = Format::Unknown;
};

[COM("1b274efe-5e37-492b-826e-7ee7e8f5a49b")]
interface IBufferResource : IResource
{
    BufferResourceDesc *getDesc();
    DeviceAddress getDeviceAddress();
    Result map(MemoryRange* rangeToRead, void** outPointer);
    Result unmap(MemoryRange* writtenRange);
};

struct DepthStencilClearValue
{
    float depth = 1.0f;
    uint32_t stencil = 0;
};

struct ColorClearValue
{
    float4 values;

    [mutating]
    void setValue(uint4 uintVal)
    {
        values = reinterpret<float4, uint4>(uintVal);
    }

    [mutating]
    void setValue(float4 floatVal)
    {
        values = floatVal;
    }
};

struct ClearValue
{
    ColorClearValue color;
    DepthStencilClearValue depthStencil;
};

struct BufferRange
{
    // TODO: Change to Index and Count?
    uint64_t firstElement;
    uint64_t elementCount;
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

    DepthStencil = 0x6,
};

struct SubresourceRange
{
    TextureAspect aspectMask;
    GfxIndex mipLevel;
    GfxCount mipLevelCount;
    GfxIndex baseArrayLayer; // For Texture3D, this is WSlice.
    GfxCount layerCount;     // For cube maps, this is a multiple of 6.
};

static const Size kRemainingTextureSize = 0xFFFFFFFF;
struct TextureResourceSampleDesc
{
    GfxCount numSamples; ///< Number of samples per pixel
    int quality;         ///< The quality measure for the samples
};

struct TextureResourceDesc : ResourceDescBase
{
    int3 size;

    GfxCount arraySize = 0; ///< Array size

    GfxCount numMipLevels = 0; ///< Number of mip levels - if 0 will create all mip levels
    Format format;             ///< The resources format
    TextureResourceSampleDesc sampleDesc;     ///< How the resource is sampled
    ClearValue* optimalClearValue;
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
    void *data;

    /// Stride in bytes between rows of the subresource tensor.
    ///
    /// This is the number of bytes to add to a pointer to a texel
    /// at (X,Y,Z) to get to a texel at (X,Y+1,Z).
    ///
    /// Devices may not support all possible values for `strideY`.
    /// In particular, they may only support strictly positive strides.
    ///
    gfx::Size strideY;

    /// Stride in bytes between layers of the subresource tensor.
    ///
    /// This is the number of bytes to add to a pointer to a texel
    /// at (X,Y,Z) to get to a texel at (X,Y,Z+1).
    ///
    /// Devices may not support all possible values for `strideZ`.
    /// In particular, they may only support strictly positive strides.
    ///
    gfx::Size strideZ;
};

[COM("cf88a31c-6187-46c5-a4b7-eb-58-c7-33-40-17")]
interface ITextureResource : IResource
{
    TextureResourceDesc* getDesc();
};

enum class ComparisonFunc : uint8_t
{
    Never = 0x0,
    Less = 0x1,
    Equal = 0x2,
    LessEqual = 0x3,
    Greater = 0x4,
    NotEqual = 0x5,
    GreaterEqual = 0x6,
    Always = 0x7,
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

struct SamplerStateDesc
{
    TextureFilteringMode minFilter;
    TextureFilteringMode magFilter;
    TextureFilteringMode mipFilter;
    TextureReductionOp reductionOp;
    TextureAddressingMode addressU;
    TextureAddressingMode addressV;
    TextureAddressingMode addressW;
    float mipLODBias;
    uint32_t maxAnisotropy;
    ComparisonFunc comparisonFunc;
    float4 borderColor;
    float minLOD;
    float maxLOD;
    __init()
    {
        minFilter = TextureFilteringMode::Linear;
        magFilter = TextureFilteringMode::Linear;
        mipFilter = TextureFilteringMode::Linear;
        reductionOp = TextureReductionOp::Average;
        addressU = TextureAddressingMode::Wrap;
        addressV = TextureAddressingMode::Wrap;
        addressW = TextureAddressingMode::Wrap;
        mipLODBias = 0.0f;
        maxAnisotropy = 1;
        comparisonFunc = ComparisonFunc::Never;
        borderColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
        minLOD = -float.maxValue;
        maxLOD = float.maxValue;
    }
};

[COM("8b8055df-9377-401d-91ff-3f-a3-bf-66-64-f4")]
interface ISamplerState
{
    /// Returns a native API handle representing this sampler state object.
    /// When using D3D12, this will be a D3D12_CPU_DESCRIPTOR_HANDLE.
    /// When using Vulkan, this will be a VkSampler.
    Result getNativeHandle(InteropHandle *outNativeHandle);
};

enum class ResourceViewType
{
    Unknown,

    RenderTarget,
    DepthStencil,
    ShaderResource,
    UnorderedAccess,
    AccelerationStructure,

    CountOf_,
};

struct RenderTargetDesc
{
    // The resource shape of this render target view.
    ResourceType shape;
};

struct ResourceViewDesc
{
    ResourceViewType type;
    Format format;

    // Required fields for `RenderTarget` and `DepthStencil` views.
    RenderTargetDesc renderTarget;
    // Specifies the range of a texture resource for a ShaderRsource/UnorderedAccess/RenderTarget/DepthStencil view.
    SubresourceRange subresourceRange;
    // Specifies the range of a buffer resource for a ShaderResource/UnorderedAccess view.
    BufferRange bufferRange;
    // Specifies the element size in bytes of a structured buffer. Pass 0 for a raw buffer view.
    Size bufferElementSize;
};

[COM("7b6c4926-0884-408c-ad8a-50-3a-8e-23-98-a4")]
interface IResourceView
{
    ResourceViewDesc* getViewDesc();

    /// Returns a native API handle representing this resource view object.
    /// When using D3D12, this will be a D3D12_CPU_DESCRIPTOR_HANDLE or a buffer device address depending
    /// on the type of the resource view.
    /// When using Vulkan, this will be a VkImageView, VkBufferView, VkAccelerationStructure or a VkBuffer
    /// depending on the type of the resource view.
    Result getNativeHandle(InteropHandle *outNativeHandle);
};

enum class AccelerationStructureKind
{
    TopLevel,
    BottomLevel
};

// The enum values are intentionally consistent with
// D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS.
enum AccelerationStructureBuildFlags
{
    None,
    AllowUpdate = 1,
    AllowCompaction = 2,
    PreferFastTrace = 4,
    PreferFastBuild = 8,
    MinimizeMemory = 16,
    PerformUpdate = 32
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
    GfxCount indexCount;
    GfxCount vertexCount;
    DeviceAddress indexData;
    DeviceAddress vertexData;
    Size vertexStride;
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
    GfxCount count;

    /// Pointer to an array of `ProceduralAABB` values in device memory.
    DeviceAddress data;

    /// Stride in bytes of the AABB values array.
    Size stride;
};

struct GeometryDesc
{
    GeometryType type;
    GeometryFlags::Enum flags;
    TriangleDesc triangles;
    property ProceduralAABBDesc proceduralAABBs
    {
        get { return reinterpret<ProceduralAABBDesc, TriangleDesc>(triangles); }
        set { triangles = reinterpret<TriangleDesc, ProceduralAABBDesc>(newValue); }
    }
};

// The enum values are kept consistent with D3D12_RAYTRACING_INSTANCE_FLAGS
// and VkGeometryInstanceFlagBitsKHR.
enum GeometryInstanceFlags
{
    None = 0,
    TriangleFacingCullDisable = 0x00000001,
    TriangleFrontCounterClockwise = 0x00000002,
    ForceOpaque = 0x00000004,
    NoOpaque = 0x00000008
};

// TODO: Should any of these be changed?
// The layout of this struct is intentionally consistent with D3D12_RAYTRACING_INSTANCE_DESC
// and VkAccelerationStructureInstanceKHR.
struct InstanceDesc
{
    float transform[3][4];
    uint32_t instanceID24_mask8;
    property uint32_t instanceID { get { return instanceID24_mask8 & 0xFFFFFF; } set { instanceID24_mask8 = (instanceID24_mask8 & 0xFF000000) | (newValue & 0xFFFFFF); } }
    property uint32_t instanceMask { get { return instanceID24_mask8 >> 24; } set { instanceID24_mask8 = (newValue << 24) | (instanceID24_mask8 & 0x00FFFFFF); } }

    uint32_t instanceContributionToHitGroupIndex24_flags8;
    property uint32_t instanceContributionToHitGroupIndex
    {
        get { return instanceContributionToHitGroupIndex24_flags8 & 0xFFFFFF; }
        set { instanceContributionToHitGroupIndex24_flags8 = (instanceContributionToHitGroupIndex24_flags8 & 0xFF000000) | (newValue & 0xFFFFFF); }
    }
    property GeometryInstanceFlags flags
    {
        get { return (GeometryInstanceFlags)(instanceContributionToHitGroupIndex24_flags8 >> 24); }
        set { instanceContributionToHitGroupIndex24_flags8 = ((uint32_t)newValue << 24) | (instanceContributionToHitGroupIndex24_flags8 & 0x00FFFFFF); }
    }
    DeviceAddress accelerationStructure;
};

struct AccelerationStructurePrebuildInfo
{
    Size resultDataMaxSize;
    Size scratchDataSize;
    Size updateScratchDataSize;
};

struct AccelerationStructureBuildInputs
{
    AccelerationStructureKind kind;

    AccelerationStructureBuildFlags flags;

    GfxCount descCount;

    /// Array of `InstanceDesc` values in device memory.
    /// Used when `kind` is `TopLevel`.
    DeviceAddress instanceDescs;

    /// Array of `GeometryDesc` values.
    /// Used when `kind` is `BottomLevel`.
    GeometryDesc *geometryDescs;
};

struct AccelerationStructureCreateDesc
{
    AccelerationStructureKind kind;
    NativeRef<IBufferResource> buffer;
    Offset offset;
    Size size;
};

struct AccelerationStructureBuildDesc
{
    AccelerationStructureBuildInputs inputs;
    NativeRef<IAccelerationStructure> source;
    NativeRef<IAccelerationStructure> dest;
    DeviceAddress scratchData;
};

[COM("a5cdda3c-1d4e-4df7-8ef2-b7-3f-ce-04-de-3b")]
interface IAccelerationStructure : IResourceView
{
    DeviceAddress getDeviceAddress();
};

struct FenceDesc
{
    uint64_t initialValue;
    bool isShared;
};

[COM("7fe1c283-d3f4-48ed-aaf3-01-51-96-4e-7c-b5")]
interface IFence
{
    /// Returns the currently signaled value on the device.
    Result getCurrentValue(uint64_t *outValue);

    /// Signals the fence from the host with the specified value.
    Result setCurrentValue(uint64_t value);

    Result getSharedHandle(InteropHandle *outHandle);
    Result getNativeHandle(InteropHandle *outNativeHandle);
};

struct ShaderOffset
{
    Int uniformOffset = 0; // TODO: Change to Offset?
    GfxIndex bindingRangeIndex = 0;
    GfxIndex bindingArrayIndex = 0;
}

enum class ShaderObjectContainerType
{
    None, Array, StructuredBuffer
};

[COM("c1fa997e-5ca2-45ae-9bcb-c4-35-9e-85-05-85")]
interface IShaderObject
{
    slang::TypeLayoutReflection* getElementTypeLayout();
    ShaderObjectContainerType getContainerType();
    GfxCount getEntryPointCount();
    Result getEntryPoint(GfxIndex index, out Optional<IShaderObject> entryPoint);
    Result setData(ShaderOffset *offset, void *data, Size size);
    Result getObject(ShaderOffset *offset, out Optional<IShaderObject> object);
    Result setObject(ShaderOffset* offset, IShaderObject object);
    Result setResource(ShaderOffset* offset, IResourceView resourceView);
    Result setSampler(ShaderOffset* offset, ISamplerState sampler);
    Result setCombinedTextureSampler(ShaderOffset* offset, IResourceView textureView, ISamplerState sampler);

    /// Manually overrides the specialization argument for the sub-object binding at `offset`.
    /// Specialization arguments are passed to the shader compiler to specialize the type
    /// of interface-typed shader parameters.
    Result setSpecializationArgs(
        ShaderOffset* offset,
        slang::SpecializationArg *args,
        GfxCount count);

    Result getCurrentVersion(
        ITransientResourceHeap transientHeap,
        out IShaderObject outObject);

    void* getRawData();

    Size getSize();

    /// Use the provided constant buffer instead of the internally created one.
    Result setConstantBufferOverride(IBufferResource constantBuffer);
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
    StencilOp stencilFailOp = StencilOp::Keep;
    StencilOp stencilDepthFailOp = StencilOp::Keep;
    StencilOp stencilPassOp = StencilOp::Keep;
    ComparisonFunc stencilFunc = ComparisonFunc::Always;
    __init()
    {
        stencilFailOp = StencilOp::Keep;
        stencilDepthFailOp = StencilOp::Keep;
        stencilPassOp = StencilOp::Keep;
        stencilFunc = ComparisonFunc::Always;
    }
};

struct DepthStencilDesc
{
    bool depthTestEnable = false;
    bool depthWriteEnable = true;
    ComparisonFunc depthFunc = ComparisonFunc::Less;

    bool stencilEnable = false;
    uint32_t stencilReadMask = 0xFFFFFFFF;
    uint32_t stencilWriteMask = 0xFFFFFFFF;
    DepthStencilOpDesc frontFace;
    DepthStencilOpDesc backFace;

    uint32_t stencilRef = 0;

    __init()
    {
        depthTestEnable = false;
        depthWriteEnable = true;
        depthFunc = ComparisonFunc::Less;
        stencilEnable = false;
        stencilReadMask = 0xFFFFFFFF;
        stencilWriteMask = 0xFFFFFFFF;
        stencilRef = 0;
    }
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

    __init()
    {
        fillMode = FillMode::Solid;
        cullMode = CullMode::None;
        frontFace = FrontFaceMode::CounterClockwise;
        depthBias = 0;
        depthBiasClamp = 0.0f;
        slopeScaledDepthBias = 0.0f;
        depthClipEnable = true;
        scissorEnable = false;
        multisampleEnable = false;
        antialiasedLineEnable = false;
        enableConservativeRasterization = false;
        forcedSampleCount = 0;
    }
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

enum RenderTargetWriteMask
{
    EnableNone = 0,
    EnableRed = 0x01,
    EnableGreen = 0x02,
    EnableBlue = 0x04,
    EnableAlpha = 0x08,
    EnableAll = 0x0F,
};

struct AspectBlendDesc
{
    BlendFactor srcFactor = BlendFactor::One;
    BlendFactor dstFactor = BlendFactor::Zero;
    BlendOp op = BlendOp::Add;

    __init()
    {
        srcFactor = BlendFactor::One;
        dstFactor = BlendFactor::Zero;
    }
};

struct TargetBlendDesc
{
    AspectBlendDesc color;
    AspectBlendDesc alpha;
    bool enableBlend;
    LogicOp logicOp;
    RenderTargetWriteMask writeMask;
    __init()
    {
        enableBlend = false;
        logicOp = LogicOp::NoOp;
        writeMask = RenderTargetWriteMask::EnableAll;
    }
};

struct BlendDesc
{
    TargetBlendDesc targets[kMaxRenderTargetCount];
    GfxCount targetCount;

    bool alphaToCoverageEnable;
};

struct FramebufferTargetLayout
{
    Format format;
    GfxCount sampleCount;
};

struct FramebufferLayoutDesc
{
    GfxCount renderTargetCount;
    FramebufferTargetLayout *renderTargets;
    FramebufferTargetLayout *depthStencil;
};

[COM("0a838785-c13a-4832-ad88-64-06-b5-4b-5e-ba")]
interface IFramebufferLayout
{
};

struct GraphicsPipelineStateDesc
{
    NativeRef<IShaderProgram> program;

    NativeRef<IInputLayout> inputLayout;
    NativeRef<IFramebufferLayout> framebufferLayout;
    PrimitiveType primitiveType;
    DepthStencilDesc depthStencil;
    RasterizerDesc rasterizer;
    BlendDesc blend;

    __init()
    {
        primitiveType = PrimitiveType::Triangle;
    }
};

struct ComputePipelineStateDesc
{
    NativeRef<IShaderProgram> program;
    void *d3d12RootSignatureOverride;
};

enum RayTracingPipelineFlags
{
    None = 0,
    SkipTriangles = 1,
    SkipProcedurals = 2,
};

struct HitGroupDesc
{
    NativeString hitGroupName;
    NativeString closestHitEntryPoint;
    NativeString anyHitEntryPoint;
    NativeString intersectionEntryPoint;
};

struct RayTracingPipelineStateDesc
{
    NativeRef<IShaderProgram> program;
    GfxCount hitGroupCount = 0;
    HitGroupDesc *hitGroups;
    int maxRecursion = 0;
    Size maxRayPayloadSize = 0;
    Size maxAttributeSizeInBytes = 8;
    RayTracingPipelineFlags flags = RayTracingPipelineFlags::None;
};

// Specifies the bytes to overwrite into a record in the shader table.
struct ShaderRecordOverwrite
{
    Offset offset;   // Offset within the shader record.
    Size size;       // Number of bytes to overwrite.
    uint8_t data[8]; // Content to overwrite.
};

struct ShaderTableDesc
{
    GfxCount rayGenShaderCount;
    NativeString* rayGenShaderEntryPointNames;
    ShaderRecordOverwrite *rayGenShaderRecordOverwrites;

    GfxCount missShaderCount;
    NativeString* missShaderEntryPointNames;
    ShaderRecordOverwrite *missShaderRecordOverwrites;

    GfxCount hitGroupCount;
    NativeString* hitGroupNames;
    ShaderRecordOverwrite *hitGroupRecordOverwrites;

    NativeRef<IShaderProgram> program;
};

[COM("a721522c-df31-4c2f-a5e7-3b-e0-12-4b-31-78")]
interface IShaderTable
{

};

[COM("0ca7e57d-8a90-44f3-bdb1-fe-9b-35-3f-5a-72")]
interface IPipelineState
{
    Result getNativeHandle(InteropHandle *outHandle);
};

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
    float minZ = 0.0f;
    float maxZ = 1.0f;
};

struct FramebufferDesc
{
    GfxCount renderTargetCount;
    NativeRef<IResourceView> *renderTargetViews;
    NativeRef<IResourceView> depthStencilView;
    NativeRef<IFramebufferLayout> layout;
};

[COM("0f0c0d9a-4ef3-4e18-9ba9-34-60-ea-69-87-95")]
interface IFramebuffer
{
};

enum class WindowHandleType
{
    Unknown,
    Win32Handle,
    XLibHandle,
};

struct WindowHandle
{
    WindowHandleType type;
    void* handleValues[2];
    static WindowHandle fromHwnd(void *hwnd)
    {
        WindowHandle handle = {};
        handle.type = WindowHandleType::Win32Handle;
        handle.handleValues[0] = hwnd;
        return handle;
    }
    static WindowHandle fromXWindow(void *xdisplay, uint32_t xwindow)
    {
        WindowHandle handle = {};
        handle.type = WindowHandleType::XLibHandle;
        handle.handleValues[0] = xdisplay;
        handle.handleValues[1] = (void*)xwindow;
        return handle;
    }
};

enum FaceMask
{
    Front = 1, Back = 2
};

enum class TargetLoadOp
{
    Load, Clear, DontCare
};
enum class TargetStoreOp
{
    Store, DontCare
};
struct TargetAccessDesc
{
    TargetLoadOp loadOp;
    TargetLoadOp stencilLoadOp;
    TargetStoreOp storeOp;
    TargetStoreOp stencilStoreOp;
    ResourceState initialState;
    ResourceState finalState;
};
struct RenderPassLayoutDesc
{
    NativeRef<IFramebufferLayout> framebufferLayout;
    GfxCount renderTargetCount;
    TargetAccessDesc *renderTargetAccess;
    TargetAccessDesc *depthStencilAccess;
};

[COM("daab0b1a-f45d-4ae9-bf2c-e0-bb-76-7d-fa-d1")]
interface IRenderPassLayout
{
};

enum class QueryType
{
    Timestamp,
    AccelerationStructureCompactedSize,
    AccelerationStructureSerializedSize,
    AccelerationStructureCurrentSize,
};

struct QueryPoolDesc
{
    QueryType type;
    GfxCount count;
};

[COM("c2cc3784-12da-480a-a874-8b-31-96-1c-a4-36")]
interface IQueryPool
{
    Result getResult(GfxIndex queryIndex, GfxCount count, uint64_t *data);
    Result reset();
};

[COM("77ea6383-be3d-40aa-8b45-fd-f0-d7-5b-fa-34")]
interface ICommandEncoder
{
    void endEncoding();
    void writeTimestamp(IQueryPool queryPool, GfxIndex queryIndex);
};

struct IndirectDispatchArguments
{
    GfxCount ThreadGroupCountX;
    GfxCount ThreadGroupCountY;
    GfxCount ThreadGroupCountZ;
};

struct IndirectDrawArguments
{
    GfxCount VertexCountPerInstance;
    GfxCount InstanceCount;
    GfxIndex StartVertexLocation;
    GfxIndex StartInstanceLocation;
};

struct IndirectDrawIndexedArguments
{
    GfxCount IndexCountPerInstance;
    GfxCount InstanceCount;
    GfxIndex StartIndexLocation;
    GfxIndex BaseVertexLocation;
    GfxIndex StartInstanceLocation;
};

struct SamplePosition
{
    int8_t x;
    int8_t y;
};

enum ClearResourceViewFlags
{
    None = 0,
    ClearDepth = 1,
    ClearStencil = 2,
    FloatClearValues = 4
};

[COM("F99A00E9-ED50-4088-8A0E-3B26755031EA")]
interface IResourceCommandEncoder : ICommandEncoder
{
    void copyBuffer(
                 IBufferResource dst,
                 Offset dstOffset,
                 IBufferResource src,
                 Offset srcOffset,
                 Size size);
    /// Copies texture from src to dst. If dstSubresource and srcSubresource has mipLevelCount = 0
    /// and layerCount = 0, the entire resource is being copied and dstOffset, srcOffset and extent
    /// arguments are ignored.
    void copyTexture(
        ITextureResource dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        int3 dstOffset,
        NativeRef<ITextureResource> src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        int3 srcOffset,
        int3 extent);

    /// Copies texture to a buffer. Each row is aligned to kTexturePitchAlignment.
    void copyTextureToBuffer(
        IBufferResource dst,
        Offset dstOffset,
        Size dstSize,
        Size dstRowStride,
        ITextureResource src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        int3 srcOffset,
        int3 extent);
    void uploadTextureData(
        ITextureResource dst,
        SubresourceRange subResourceRange,
        int3 offset,
        int3 extent,
        SubresourceData *subResourceData,
        GfxCount subResourceDataCount);
    void uploadBufferData(IBufferResource dst, Offset offset, Size size, void *data);
    void textureBarrier(
        GfxCount count, NativeRef<ITextureResource> *textures, ResourceState src, ResourceState dst);
    void textureSubresourceBarrier(
        ITextureResource texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst);
    void bufferBarrier(
        GfxCount count, NativeRef<IBufferResource>* buffers, ResourceState src, ResourceState dst);
    void clearResourceView(
        IResourceView view, ClearValue *clearValue, ClearResourceViewFlags flags);
    void resolveResource(
        ITextureResource source,
        ResourceState sourceState,
        SubresourceRange sourceRange,
        ITextureResource dest,
        ResourceState destState,
        SubresourceRange destRange);
    void resolveQuery(
        IQueryPool queryPool,
        GfxIndex index,
        GfxCount count,
        IBufferResource buffer,
        Offset offset);
    void beginDebugEvent(NativeString name, float rgbColor[3]);
    void endDebugEvent();
};

[COM("7A8D56D0-53E6-4AD6-85F7-D14DC110FDCE")]
interface IRenderCommandEncoder : IResourceCommandEncoder
{
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRootShaderObject` stays alive during
    // the execution of the command buffer.
    Result bindPipeline(IPipelineState state, out IShaderObject outRootShaderObject);

    // Sets the current pipeline state along with a pre-created mutable root shader object.
    Result bindPipelineWithRootObject(IPipelineState state, NativeRef<IShaderObject> rootObject);

    void setViewports(GfxCount count, Viewport *viewports);
    void setScissorRects(GfxCount count, ScissorRect *scissors);

    void setPrimitiveTopology(PrimitiveTopology topology);
    void setVertexBuffers(
        GfxIndex startSlot,
        GfxCount slotCount,
        NativeRef<IBufferResource>* buffers,
        Offset *offsets);

    void setIndexBuffer(IBufferResource buffer, Format indexFormat, Offset offset);
    void draw(GfxCount vertexCount, GfxIndex startVertex);
    void drawIndexed(GfxCount indexCount, GfxIndex startIndex = 0, GfxIndex baseVertex = 0);
    void drawIndirect(
        GfxCount maxDrawCount,
        IBufferResource argBuffer,
        Offset argOffset,
        NativeRef<IBufferResource> countBuffer,
        Offset countOffset = 0);
    void drawIndexedIndirect(
        GfxCount maxDrawCount,
        IBufferResource argBuffer,
        Offset argOffset,
        NativeRef<IBufferResource> countBuffer,
        Offset countOffset = 0);
    void setStencilReference(uint32_t referenceValue);
    Result setSamplePositions(
        GfxCount samplesPerPixel, GfxCount pixelCount, SamplePosition *samplePositions);
    void drawInstanced(
        GfxCount vertexCount,
        GfxCount instanceCount,
        GfxIndex startVertex,
        GfxIndex startInstanceLocation);
    void drawIndexedInstanced(
        GfxCount indexCount,
        GfxCount instanceCount,
        GfxIndex startIndexLocation,
        GfxIndex baseVertexLocation,
        GfxIndex startInstanceLocation);
};

[COM("88AA9322-82F7-4FE6-A68A-29C7FE798737")]
interface IComputeCommandEncoder : IResourceCommandEncoder
{
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRooShaderObject` stays alive during
    // the execution of the command buffer.
    Result bindPipeline(IPipelineState state, out Optional<IShaderObject> outRootShaderObject);
    
    // Sets the current pipeline state along with a pre-created mutable root shader object.
    Result bindPipelineWithRootObject(IPipelineState state, IShaderObject rootObject);

    void dispatchCompute(int x, int y, int z);
    void dispatchComputeIndirect(IBufferResource cmdBuffer, Offset offset);
};

enum class AccelerationStructureCopyMode
{
    Clone, Compact
};

struct AccelerationStructureQueryDesc
{
    QueryType queryType;

    NativeRef<IQueryPool> queryPool;

    GfxIndex firstQueryIndex;
};

[COM("9a672b87-5035-45e3-967c-1f-85-cd-b3-63-4f")]
interface IRayTracingCommandEncoder : IResourceCommandEncoder
{
    void buildAccelerationStructure(
                 AccelerationStructureBuildDesc *desc,
                 GfxCount propertyQueryCount,
                 AccelerationStructureQueryDesc *queryDescs);
    void copyAccelerationStructure(
        NativeRef<IAccelerationStructure> dest,
        NativeRef<IAccelerationStructure> src,
        AccelerationStructureCopyMode mode);
    void queryAccelerationStructureProperties(
        GfxCount accelerationStructureCount,
        NativeRef<IAccelerationStructure> *accelerationStructures,
        GfxCount queryCount,
        AccelerationStructureQueryDesc *queryDescs);
    void serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure source);
    void deserializeAccelerationStructure(IAccelerationStructure dest, DeviceAddress source);

    void bindPipeline(IPipelineState state, out IShaderObject rootObject);
    // Sets the current pipeline state along with a pre-created mutable root shader object.
    Result bindPipelineWithRootObject(IPipelineState state, IShaderObject rootObject);

    /// Issues a dispatch command to start ray tracing workload with a ray tracing pipeline.
    /// `rayGenShaderIndex` specifies the index into the shader table that identifies the ray generation shader.
    void dispatchRays(
        GfxIndex rayGenShaderIndex,
        NativeRef<IShaderTable> shaderTable,
        GfxCount width,
        GfxCount height,
        GfxCount depth);
};

[COM("5d56063f-91d4-4723-a7a7-7a-15-af-93-eb-48")]
interface ICommandBuffer
{
    // Only one encoder may be open at a time. User must call `ICommandEncoder::endEncoding`
    // before calling other `encode*Commands` methods.
    // Once `endEncoding` is called, the `ICommandEncoder` object becomes obsolete and is
    // invalid for further use. To continue recording, the user must request a new encoder
    // object by calling one of the `encode*Commands` methods again.
    void encodeRenderCommands(
        IRenderPassLayout renderPass,
        IFramebuffer framebuffer,
        out IRenderCommandEncoder outEncoder);

    void encodeComputeCommands(out Optional<IComputeCommandEncoder> encoder);

    void encodeResourceCommands(out Optional<IResourceCommandEncoder> outEncoder);

    void encodeRayTracingCommands(out Optional<IRayTracingCommandEncoder> outEncoder);

    void close();

    Result getNativeHandle(out InteropHandle outHandle);
};

enum class QueueType
{
    Graphics
};
struct CommandQueueDesc
{
    QueueType type;
};

[COM("14e2bed0-0ad0-4dc8-b341-06-3f-e7-2d-bf-0e")]
interface ICommandQueue
{
    const CommandQueueDesc* getDesc();

    void executeCommandBuffers(
        GfxCount count,
        NativeRef<ICommandBuffer> *commandBuffers,
        Optional<IFence> fenceToSignal,
        uint64_t newFenceValue);

    Result getNativeHandle(out InteropHandle outHandle);

    void waitOnHost();

    /// Queues a device side wait for the given fences.
    Result waitForFenceValuesOnDevice(GfxCount fenceCount, NativeRef<IFence> *fences, uint64_t *waitValues);
};

enum TransientResourceHeapFlags
{
    None = 0,
    AllowResizing = 0x1,
};

struct TransientResourceHeapDesc
{
    TransientResourceHeapFlags flags;
    Size constantBufferSize;
    GfxCount samplerDescriptorCount;
    GfxCount uavDescriptorCount;
    GfxCount srvDescriptorCount;
    GfxCount constantBufferDescriptorCount;
    GfxCount accelerationStructureDescriptorCount;
};

[COM("cd48bd29-ee72-41b8-bcff-0a-2b-3a-aa-6d-0b")]
interface ITransientResourceHeap
{
    // Waits until GPU commands issued before last call to `finish()` has been completed, and resets
    // all transient resources holds by the heap.
    // This method must be called before using the transient heap to issue new GPU commands.
    // In most situations this method should be called at the beginning of each frame.
    Result synchronizeAndReset();

    // Must be called when the application has done using this heap to issue commands. In most situations
    // this method should be called at the end of each frame.
    Result finish();

    // Command buffers are one-time use. Once it is submitted to the queue via
    // `executeCommandBuffers` a command buffer is no longer valid to be used any more. Command
    // buffers must be closed before submission. The current D3D12 implementation has a limitation
    // that only one command buffer maybe recorded at a time. User must finish recording a command
    // buffer before creating another command buffer.
    Result createCommandBuffer(out Optional<ICommandBuffer> outCommandBuffer);
};

struct SwapchainDesc
{
    Format format;
    GfxCount width, height;
    GfxCount imageCount;
    NativeRef<ICommandQueue> queue;
    bool enableVSync;
};

[COM("be91ba6c-0784-4308-a1-00-19-c3-66-83-44-b2")]
interface ISwapchain
{
    const SwapchainDesc* getDesc();

    /// Returns the back buffer image at `index`.
    Result getImage(GfxIndex index, out ITextureResource outResource);

    /// Present the next image in the swapchain.
    Result present();

    /// Returns the index of next back buffer image that will be presented in the next
    /// `present` call. If the swapchain is invalid/out-of-date, this method returns -1.
    int acquireNextImage();

    /// Resizes the back buffers of this swapchain. All render target views and framebuffers
    /// referencing the back buffer images must be freed before calling this method.
    Result resize(GfxCount width, GfxCount height);

    // Check if the window is occluded.
    bool isOccluded();

    // Toggle full screen mode.
    Result setFullScreenMode(bool mode);
};

struct DeviceInfo
{
    DeviceType deviceType;

    BindingStyle bindingStyle;

    ProjectionStyle projectionStyle;

    /// An projection matrix that ensures x, y mapping to pixels
    /// is the same on all targets
    float identityProjectionMatrix[16];

    /// The name of the graphics API being used by this device.
    NativeString apiName;

    /// The name of the graphics adapter.
    NativeString adapterName;

    /// The clock frequency used in timestamp queries.
    uint64_t timestampFrequency;
};

enum class DebugMessageType
{
    Info, Warning, Error
};
enum class DebugMessageSource
{
    Layer, Driver, Slang
};

[COM("B219D7E8-255A-2572-D46C-A0E5D99CEB90")]
interface IDebugCallback
{
    void handleMessage(DebugMessageType type, DebugMessageSource source, NativeString message);
};

struct SlangDesc
{
    NativeRef<slang::IGlobalSession> slangGlobalSession; // (optional) A slang global session object. If null will create automatically.

    slang::SlangMatrixLayoutMode defaultMatrixLayoutMode = slang::SlangMatrixLayoutMode::SLANG_MATRIX_LAYOUT_ROW_MAJOR;

    NativeString *searchPaths;
    GfxCount searchPathCount;

    slang::PreprocessorMacroDesc* preprocessorMacros;
    GfxCount preprocessorMacroCount = 0;

    NativeString targetProfile; // (optional) Target shader profile. If null this will be set to platform dependent default.
    slang::SlangFloatingPointMode floatingPointMode = slang::SlangFloatingPointMode::SLANG_FLOATING_POINT_MODE_DEFAULT;
    slang::SlangOptimizationLevel optimizationLevel = slang::SlangOptimizationLevel::SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    slang::SlangTargetFlags targetFlags = slang::SlangTargetFlags.None;
    slang::SlangLineDirectiveMode lineDirectiveMode = slang::SlangLineDirectiveMode::SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
};

struct ShaderCacheDesc
{
    // The root directory for the shader cache. If not set, shader cache is disabled.
    NativeString shaderCachePath;
    // The maximum number of entries stored in the cache.
    GfxCount maxEntryCount = 0;
};

struct DeviceInteropHandles
{
    InteropHandle handles[3] = {};
};

struct DeviceDesc
{
    // The underlying API/Platform of the device.
    DeviceType deviceType = DeviceType::Default;
    // The device's handles (if they exist) and their associated API. For D3D12, this contains a single InteropHandle
    // for the ID3D12Device. For Vulkan, the first InteropHandle is the VkInstance, the second is the VkPhysicalDevice,
    // and the third is the VkDevice. For CUDA, this only contains a single value for the CUDADevice.
    DeviceInteropHandles existingDeviceHandles;
    // Name to identify the adapter to use
    NativeString adapter;
    // Number of required features.
    GfxCount requiredFeatureCount = 0;
    // Array of required feature names, whose size is `requiredFeatureCount`.
    NativeString *requiredFeatures = nullptr;
    // A command dispatcher object that intercepts and handles actual low-level API call.
    void *apiCommandDispatcher = nullptr;
    // The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
    GfxIndex nvapiExtnSlot = -1;
    // Configurations for the shader cache.
    ShaderCacheDesc shaderCache = {};
    // Configurations for Slang compiler.
    SlangDesc slang = {};

    GfxCount extendedDescCount = 0;
    void **extendedDescs = nullptr;
};

[COM("715bdf26-5135-11eb-AE93-02-42-AC-13-00-02")]
interface IDevice
{
    Result getNativeDeviceHandles(out DeviceInteropHandles outHandles);

    bool hasFeature(NativeString feature);

    /// Returns a list of features supported by the renderer.
    Result getFeatures(NativeString *outFeatures, Size bufferSize, GfxCount *outFeatureCount);

    Result getFormatSupportedResourceStates(Format format, ResourceStateSet *outStates);

    Result getSlangSession(NativeRef<slang::ISession>* outSlangSession);

    Result createTransientResourceHeap(
        TransientResourceHeapDesc *desc,
        out Optional<ITransientResourceHeap> outHeap);

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
    Result createTextureResource(
        TextureResourceDesc* desc,
        SubresourceData *initData,
        out ITextureResource outResource);

    Result createTextureFromNativeHandle(
        InteropHandle handle,
        TextureResourceDesc* srcDesc,
        out ITextureResource outResource);

    Result createTextureFromSharedHandle(
        InteropHandle handle,
        TextureResourceDesc *srcDesc,
        Size size,
        out ITextureResource outResource);

    /// Create a buffer resource
    Result createBufferResource(
        BufferResourceDesc* desc,
        void *initData,
        out Optional<IBufferResource> outResource);

    Result createBufferFromNativeHandle(
        InteropHandle handle,
        BufferResourceDesc* srcDesc,
        out IBufferResource outResource);

    Result createBufferFromSharedHandle(
        InteropHandle handle,
        BufferResourceDesc* srcDesc,
        out IBufferResource outResource);

    Result createSamplerState(SamplerStateDesc* desc, out ISamplerState outSampler);

    Result createTextureView(
        ITextureResource texture, ResourceViewDesc* desc, out IResourceView outView);

    Result createBufferView(
        IBufferResource buffer,
        Optional<IBufferResource> counterBuffer,
        ResourceViewDesc* desc,
        out Optional<IResourceView> outView);

    Result createFramebufferLayout(FramebufferLayoutDesc* desc, out IFramebufferLayout outFrameBuffer);
    
    Result createFramebuffer(FramebufferDesc* desc, out IFramebuffer outFrameBuffer);

    Result createRenderPassLayout(
        RenderPassLayoutDesc* desc,
        out IRenderPassLayout outRenderPassLayout);

    Result createSwapchain(
        SwapchainDesc* desc, WindowHandle window, out ISwapchain outSwapchain);

    Result createInputLayout(
        InputLayoutDesc* desc, out IInputLayout outLayout);

    Result createCommandQueue(CommandQueueDesc* desc, out Optional<ICommandQueue> outQueue);

    Result createShaderObject(
        slang::TypeReflection *type,
        ShaderObjectContainerType container,
        out IShaderObject outObject);

    Result createMutableShaderObject(
        slang::TypeReflection *type,
        ShaderObjectContainerType container,
        out IShaderObject outObject);

    Result createShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection *typeLayout, out IShaderObject outObject);

    Result createMutableShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection *typeLayout, out IShaderObject outObject);

    Result createMutableRootShaderObject(
        IShaderProgram program,
        out IShaderObject outObject);

    Result createShaderTable(ShaderTableDesc* desc, out IShaderTable outTable);

    Result createProgram(
        void *desc,
        out IShaderProgram outProgram,
        out slang::ISlangBlob outDiagnosticBlob);

    Result createProgram2(
        ShaderProgramDesc2 *desc,
        out Optional<IShaderProgram> outProgram,
        out Optional<slang::ISlangBlob> outDiagnosticBlob);

    Result createGraphicsPipelineState(
        GraphicsPipelineStateDesc *desc,
        out Optional<IPipelineState> outState);

    Result createComputePipelineState(
        ComputePipelineStateDesc* desc,
        out Optional<IPipelineState> outState);

    Result createRayTracingPipelineState(
        RayTracingPipelineStateDesc *desc, out Optional<IPipelineState> outState);

    /// Read back texture resource and stores the result in `outBlob`.
    Result readTextureResource(
        ITextureResource resource,
        ResourceState state,
        out slang::ISlangBlob outBlob,
        out Size outRowPitch,
        out Size outPixelSize);

    Result readBufferResource(
        IBufferResource buffer,
        Offset offset,
        Size size,
        out Optional<slang::ISlangBlob> outBlob);

    /// Get the type of this renderer
    DeviceInfo* getDeviceInfo();

    Result createQueryPool(
        QueryPoolDesc* desc, out IQueryPool outPool);

    Result getAccelerationStructurePrebuildInfo(
        AccelerationStructureBuildInputs* buildInputs,
        out AccelerationStructurePrebuildInfo outPrebuildInfo);

    Result createAccelerationStructure(
        AccelerationStructureCreateDesc* desc,
        out IAccelerationStructure outView);

    Result createFence(FenceDesc* desc, out IFence outFence);

    /// Wait on the host for the fences to signals.
    /// `timeout` is in nanoseconds, can be set to `kTimeoutInfinite`.
    Result waitForFences(
        GfxCount fenceCount,
        NativeRef<IFence>* fences,
        uint64_t *values,
        bool waitForAll,
        uint64_t timeout);

    Result getTextureAllocationInfo(
        TextureResourceDesc* desc, out Size outSize, out Size outAlignment);

    Result getTextureRowAlignment(out Size outAlignment);
};

struct ShaderCacheStats
{
    GfxCount hitCount;
    GfxCount missCount;
    GfxCount entryCount;
};

[COM("715bdf26-5135-11eb-AE93-02-42-AC-13-00-02")]
interface IShaderCache
{
    Result clearShaderCache();
    Result getShaderCacheStats(out ShaderCacheStats outStats);
    Result resetShaderCacheStats();
};

#define SLANG_GFX_IMPORT [DllImport("gfx")]
/// Checks if format is compressed
SLANG_GFX_IMPORT bool gfxIsCompressedFormat(Format format);

/// Checks if format is typeless
SLANG_GFX_IMPORT bool gfxIsTypelessFormat(Format format);

/// Gets information about the format
SLANG_GFX_IMPORT Result gfxGetFormatInfo(Format format, FormatInfo *outInfo);

/// Given a type returns a function that can construct it, or nullptr if there isn't one
SLANG_GFX_IMPORT Result gfxCreateDevice(const DeviceDesc* desc, out Optional<IDevice> outDevice);

/// Reports current set of live objects in gfx.
/// Currently this only calls D3D's ReportLiveObjects.
SLANG_GFX_IMPORT Result gfxReportLiveObjects();

/// Sets a callback for receiving debug messages.
/// The layer does not hold a strong reference to the callback object.
/// The user is responsible for holding the callback object alive.
SLANG_GFX_IMPORT Result gfxSetDebugCallback(IDebugCallback callback);

/// Enables debug layer. The debug layer will check all `gfx` calls and verify that uses are valid.
SLANG_GFX_IMPORT void gfxEnableDebugLayer();

SLANG_GFX_IMPORT NativeString gfxGetDeviceTypeName(DeviceType type);

public bool succeeded(Result code)
{
    return code >= 0;
}

}
