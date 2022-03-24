#include "debug-layer.h"
#include "renderer-shared.h"
#include "slang-gfx.h"

using namespace Slang;

namespace gfx
{
#ifdef __FUNCSIG__
#    define SLANG_FUNC_SIG __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)
#    define SLANG_FUNC_SIG __FUNCSIG__
#elif defined(__FUNCTION__)
#    define SLANG_FUNC_SIG __FUNCTION__
#else
#    define SLANG_FUNC_SIG "UnknownFunction"
#endif

thread_local const char* _currentFunctionName = nullptr;
struct SetCurrentFuncRAII
{
    SetCurrentFuncRAII(const char* funcName) { _currentFunctionName = funcName; }
    ~SetCurrentFuncRAII() { _currentFunctionName = nullptr; }
};
#define SLANG_GFX_API_FUNC SetCurrentFuncRAII setFuncNameRAII(SLANG_FUNC_SIG)
#define SLANG_GFX_API_FUNC_NAME(x) SetCurrentFuncRAII setFuncNameRAII(x)

/// Returns the public API function name from a `SLANG_FUNC_SIG` string.
String _gfxGetFuncName(const char* input)
{
    UnownedStringSlice str(input);
    auto prefixIndex = str.indexOf(UnownedStringSlice("Debug"));
    if (prefixIndex == -1)
        return input;
    auto endIndex = str.lastIndexOf('(');
    if (endIndex == -1)
        endIndex = str.getLength();
    auto startIndex = prefixIndex + 5;
    StringBuilder sb;
    sb.appendChar('I');
    sb.append(str.subString(startIndex, endIndex - startIndex));
    return sb.ProduceString();
}

template <typename... TArgs>
static char* _gfxDiagnoseFormat(
    char* buffer, // Initial buffer to output formatted string.
    size_t shortBufferSize, // Size of the initial buffer.
    List<char>& bufferArray, // A list for allocating a large buffer if needed.
    const char* format, // The format string.
    TArgs... args)
{
    int length = sprintf_s(buffer, shortBufferSize, format, args...);
    if (length < 0)
        return buffer;
    if (length > 255)
    {
        bufferArray.setCount(length + 1);
        buffer = bufferArray.getBuffer();
        sprintf_s(buffer, bufferArray.getCount(), format, args...);
    }
    return buffer;
}

template <typename... TArgs>
static void _gfxDiagnoseImpl(DebugMessageType type, const char* format, TArgs... args)
{
    char shortBuffer[256];
    List<char> bufferArray;
    auto buffer =
        _gfxDiagnoseFormat(shortBuffer, sizeof(shortBuffer), bufferArray, format, args...);
    getDebugCallback()->handleMessage(type, DebugMessageSource::Layer, buffer);
}

#define GFX_DIAGNOSE_ERROR(message)                                                                \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Error,                                                                   \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_WARNING(message)                                                              \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Warning,                                                                 \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_INFO(message)                                                                 \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Info,                                                                    \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_FORMAT(type, format, ...)                                            \
    {                                                                                     \
        char shortBuffer[256];                                                            \
        List<char> bufferArray;                                                           \
        auto message = _gfxDiagnoseFormat(                                                \
            shortBuffer, sizeof(shortBuffer), bufferArray, format, __VA_ARGS__);          \
        _gfxDiagnoseImpl(                                                                 \
            type,                                                                         \
            "%s: %s",                                                                     \
            _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG) \
                .getBuffer(),                                                             \
            message);                                                                     \
    }
#define GFX_DIAGNOSE_ERROR_FORMAT(...) GFX_DIAGNOSE_FORMAT(DebugMessageType::Error, __VA_ARGS__)

#define SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(typeName)                                    \
    I##typeName* Debug##typeName::getInterface(const Slang::Guid& guid)                 \
    {                                                                                   \
        return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_I##typeName) \
                   ? static_cast<I##typeName*>(this)                                    \
                   : nullptr;                                                           \
    }
#define SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(typeName, parentType)                   \
    I##typeName* Debug##typeName::getInterface(const Slang::Guid& guid)                   \
    {                                                                                     \
        return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_I##typeName || \
                guid == GfxGUID::IID_I##parentType)                                       \
                   ? static_cast<I##typeName*>(this)                                      \
                   : nullptr;                                                             \
    }

SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Device)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(BufferResource, Resource)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(TextureResource, Resource)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(CommandBuffer)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(CommandQueue)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Framebuffer)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(FramebufferLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(InputLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(RenderPassLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(PipelineState)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ResourceView)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(SamplerState)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderObject)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderProgram)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Swapchain)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(TransientResourceHeap)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(QueryPool)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(AccelerationStructure, ResourceView)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Fence)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderTable)

#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL
#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT

// Utility conversion functions to get Debug* object or the inner object from a user provided
// pointer.
#define SLANG_GFX_DEBUG_GET_OBJ_IMPL(type)                                                   \
    static Debug##type* getDebugObj(I##type* ptr) { return static_cast<Debug##type*>(ptr); } \
    static I##type* getInnerObj(I##type* ptr)                                                \
    {                                                                                        \
        if (!ptr) return nullptr;                                                            \
        auto debugObj = getDebugObj(ptr);                                                    \
        return debugObj->baseObject;                                                         \
    }

SLANG_GFX_DEBUG_GET_OBJ_IMPL(Device)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(BufferResource)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(TextureResource)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(CommandBuffer)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(CommandQueue)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ComputeCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RenderCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ResourceCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RayTracingCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Framebuffer)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(FramebufferLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(InputLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RenderPassLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(PipelineState)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ResourceView)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(SamplerState)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderObject)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderProgram)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Swapchain)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(TransientResourceHeap)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(QueryPool)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(AccelerationStructure)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Fence)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderTable)

#undef SLANG_GFX_DEBUG_GET_OBJ_IMPL

void validateAccelerationStructureBuildInputs(
    const IAccelerationStructure::BuildInputs& buildInputs)
{
    switch (buildInputs.kind)
    {
    case IAccelerationStructure::Kind::TopLevel:
        if (!buildInputs.instanceDescs)
        {
            GFX_DIAGNOSE_ERROR("IAccelerationStructure::BuildInputs::instanceDescs cannot be null "
                               "when creating a top-level acceleration structure.");
        }
        break;
    case IAccelerationStructure::Kind::BottomLevel:
        if (!buildInputs.geometryDescs)
        {
            GFX_DIAGNOSE_ERROR("IAccelerationStructure::BuildInputs::geometryDescs cannot be null "
                               "when creating a bottom-level acceleration structure.");
        }
        for (int i = 0; i < buildInputs.descCount; i++)
        {
            switch (buildInputs.geometryDescs[i].type)
            {
            case IAccelerationStructure::GeometryType::Triangles:
                switch (buildInputs.geometryDescs[i].content.triangles.vertexFormat)
                {
                case Format::R32G32B32_FLOAT:
                case Format::R32G32_FLOAT:
                case Format::R16G16B16A16_FLOAT:
                case Format::R16G16_FLOAT:
                case Format::R16G16B16A16_SNORM:
                case Format::R16G16_SNORM:
                    break;
                default:
                    GFX_DIAGNOSE_ERROR(
                        "Unsupported IAccelerationStructure::TriangleDesc::vertexFormat. Valid "
                        "values are R32G32B32_FLOAT, R32G32_FLOAT, R16G16B16A16_FLOAT, R16G16_FLOAT, "
                        "R16G16B16A16_SNORM or R16G16_SNORM.");
                }
                if (buildInputs.geometryDescs[i].content.triangles.indexCount)
                {
                    switch (buildInputs.geometryDescs[i].content.triangles.indexFormat)
                    {
                    case Format::R32_UINT:
                    case Format::R16_UINT:
                        break;
                    default:
                        GFX_DIAGNOSE_ERROR(
                            "Unsupported IAccelerationStructure::TriangleDesc::indexFormat. Valid "
                            "values are Unknown, R32_UINT or R16_UINT.");
                    }
                    if (!buildInputs.geometryDescs[i].content.triangles.indexData)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData cannot be null if "
                            "IAccelerationStructure::TriangleDesc::indexCount is not 0");
                    }
                }
                if (buildInputs.geometryDescs[i].content.triangles.indexFormat != Format::Unknown)
                {
                    if (buildInputs.geometryDescs[i].content.triangles.indexCount == 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexCount cannot be 0 if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is not Format::Unknown");
                    }
                    if (buildInputs.geometryDescs[i].content.triangles.indexData == 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData cannot be null if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is not "
                            "Format::Unknown");
                    }
                }
                else
                {
                    if (buildInputs.geometryDescs[i].content.triangles.indexCount != 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexCount must be 0 if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is "
                            "Format::Unknown");
                    }
                    if (buildInputs.geometryDescs[i].content.triangles.indexData != 0)
                    {
                        GFX_DIAGNOSE_ERROR(
                            "IAccelerationStructure::TriangleDesc::indexData must be null if "
                            "IAccelerationStructure::TriangleDesc::indexFormat is "
                            "Format::Unknown");
                    }
                }
                if (!buildInputs.geometryDescs[i].content.triangles.vertexData)
                {
                    GFX_DIAGNOSE_ERROR(
                        "IAccelerationStructure::TriangleDesc::vertexData cannot be null.");
                }
                break;
            }
        }
        break;
    default:
        GFX_DIAGNOSE_ERROR("Invalid value of IAccelerationStructure::Kind.");
        break;
    }
}

Result DebugDevice::getNativeDeviceHandles(InteropHandles* outHandles)
{
    return baseObject->getNativeDeviceHandles(outHandles);
}

Result DebugDevice::getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getFeatures(outFeatures, bufferSize, outFeatureCount);
}

Result DebugDevice::getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getFormatSupportedResourceStates(format, outStates);
}

DebugDevice::DebugDevice()
{
    SLANG_GFX_API_FUNC_NAME("CreateDevice");
    GFX_DIAGNOSE_INFO("Debug layer is enabled.");
}

SLANG_NO_THROW bool SLANG_MCALL DebugDevice::hasFeature(const char* feature)
{
    SLANG_GFX_API_FUNC;

    return baseObject->hasFeature(feature);
}

Result DebugDevice::getSlangSession(slang::ISession** outSlangSession)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getSlangSession(outSlangSession);
}

Result DebugDevice::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc,
    ITransientResourceHeap** outHeap)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugTransientResourceHeap> outObject = new DebugTransientResourceHeap();
    auto result = baseObject->createTransientResourceHeap(desc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outHeap, outObject);
    return result;
}

Result DebugDevice::createTextureResource(
    const ITextureResource::Desc& desc,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugTextureResource> outObject = new DebugTextureResource();
    auto result =
        baseObject->createTextureResource(desc, initData, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createTextureFromNativeHandle(
    InteropHandle handle,
    const ITextureResource::Desc& srcDesc,
    ITextureResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugTextureResource> outObject = new DebugTextureResource();
    auto result = baseObject->createTextureFromNativeHandle(handle, srcDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createTextureFromSharedHandle(
    InteropHandle handle,
    const ITextureResource::Desc& srcDesc,
    const size_t size,
    ITextureResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugTextureResource> outObject = new DebugTextureResource();
    auto result = baseObject->createTextureFromSharedHandle(handle, srcDesc, size, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createBufferResource(
    const IBufferResource::Desc& desc,
    const void* initData,
    IBufferResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugBufferResource> outObject = new DebugBufferResource();
    auto result = baseObject->createBufferResource(desc, initData, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createBufferFromNativeHandle(
    InteropHandle handle,
    const IBufferResource::Desc& srcDesc,
    IBufferResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugBufferResource> outObject = new DebugBufferResource();
    auto result = baseObject->createBufferFromNativeHandle(handle, srcDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createBufferFromSharedHandle(
    InteropHandle handle,
    const IBufferResource::Desc& srcDesc,
    IBufferResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugBufferResource> outObject = new DebugBufferResource();
    auto result = baseObject->createBufferFromSharedHandle(handle, srcDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outResource, outObject);
    return result;
}

Result DebugDevice::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugSamplerState> outObject = new DebugSamplerState();
    auto result = baseObject->createSamplerState(desc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outSampler, outObject);
    return result;
}

Result DebugDevice::createTextureView(
    ITextureResource* texture,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugResourceView> outObject = new DebugResourceView();
    auto result = baseObject->createTextureView(
        static_cast<DebugTextureResource*>(texture)->baseObject,
        desc,
        outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outView, outObject);
    return result;
}

Result DebugDevice::createBufferView(
    IBufferResource* buffer,
    IBufferResource* counterBuffer,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugResourceView> outObject = new DebugResourceView();
    auto result = baseObject->createBufferView(
        static_cast<DebugBufferResource*>(buffer)->baseObject,
        counterBuffer ? static_cast<DebugBufferResource*>(counterBuffer)->baseObject : nullptr,
        desc,
        outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outView, outObject);
    return result;
}

Result DebugDevice::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    SLANG_GFX_API_FUNC;
    validateAccelerationStructureBuildInputs(buildInputs);
    return baseObject->getAccelerationStructurePrebuildInfo(buildInputs, outPrebuildInfo);
}

Result DebugDevice::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc,
    IAccelerationStructure** outAS)
{
    SLANG_GFX_API_FUNC;
    auto innerDesc = desc;
    innerDesc.buffer = getInnerObj(innerDesc.buffer);
    RefPtr<DebugAccelerationStructure> outObject = new DebugAccelerationStructure();
    auto result = baseObject->createAccelerationStructure(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outAS, outObject);
    return SLANG_OK;
}

Result DebugDevice::createFramebufferLayout(
    IFramebufferLayout::Desc const& desc,
    IFramebufferLayout** outFrameBuffer)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugFramebufferLayout> outObject = new DebugFramebufferLayout();
    auto result = baseObject->createFramebufferLayout(desc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outFrameBuffer, outObject);
    return result;
}

Result DebugDevice::createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer)
{
    SLANG_GFX_API_FUNC;

    auto innerDesc = desc;
    innerDesc.layout =
        desc.layout ? static_cast<DebugFramebufferLayout*>(desc.layout)->baseObject.get() : nullptr;
    innerDesc.depthStencilView =
        desc.depthStencilView
            ? static_cast<DebugResourceView*>(desc.depthStencilView)->baseObject.get()
            : nullptr;
    List<IResourceView*> innerRenderTargets;
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        auto innerRenderTarget =
            desc.renderTargetViews[i]
                ? static_cast<DebugResourceView*>(desc.renderTargetViews[i])->baseObject.get()
                : nullptr;
        innerRenderTargets.add(innerRenderTarget);
    }
    innerDesc.renderTargetViews = innerRenderTargets.getBuffer();

    RefPtr<DebugFramebuffer> outObject = new DebugFramebuffer();
    auto result = baseObject->createFramebuffer(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outFrameBuffer, outObject);
    return result;
}

Result DebugDevice::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    SLANG_GFX_API_FUNC;

    auto innerDesc = desc;
    innerDesc.framebufferLayout =
        desc.framebufferLayout? static_cast<DebugFramebufferLayout*>(desc.framebufferLayout)->baseObject.get()
            : nullptr;
    RefPtr<DebugRenderPassLayout> outObject = new DebugRenderPassLayout();
    auto result = baseObject->createRenderPassLayout(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outRenderPassLayout, outObject);
    return result;
}

Result DebugDevice::createSwapchain(
    ISwapchain::Desc const& desc,
    WindowHandle window,
    ISwapchain** outSwapchain)
{
    SLANG_GFX_API_FUNC;

    auto innerDesc = desc;
    innerDesc.queue = static_cast<DebugCommandQueue*>(desc.queue)->baseObject.get();
    RefPtr<DebugSwapchain> outObject = new DebugSwapchain();
    outObject->queue = static_cast<DebugCommandQueue*>(desc.queue);
    auto result = baseObject->createSwapchain(innerDesc, window, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outSwapchain, outObject);
    return Result();
}

Result DebugDevice::createInputLayout(
    IInputLayout::Desc const& desc,
    IInputLayout** outLayout)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugInputLayout> outObject = new DebugInputLayout();
    auto result = baseObject->createInputLayout(
        desc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outLayout, outObject);
    return result;
}

Result DebugDevice::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugCommandQueue> outObject = new DebugCommandQueue();
    auto result = baseObject->createCommandQueue(desc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outQueue, outObject);
    return result;
}

Result DebugDevice::createShaderObject(
    slang::TypeReflection* type,
    ShaderObjectContainerType containerType,
    IShaderObject** outShaderObject)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto typeName = type->getName();
    auto result =
        baseObject->createShaderObject(type, containerType, outObject->baseObject.writeRef());
    outObject->m_typeName = typeName;
    outObject->m_device = this;
    outObject->m_slangType = type;
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outShaderObject, outObject);
    return result;
}

Result DebugDevice::createMutableShaderObject(
    slang::TypeReflection* type,
    ShaderObjectContainerType containerType,
    IShaderObject** outShaderObject)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto typeName = type->getName();
    auto result =
        baseObject->createMutableShaderObject(type, containerType, outObject->baseObject.writeRef());
    outObject->m_typeName = typeName;
    outObject->m_device = this;
    outObject->m_slangType = type;
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outShaderObject, outObject);
    return result;
}

Result DebugDevice::createMutableRootShaderObject(
    IShaderProgram* program, IShaderObject** outRootObject)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto result = baseObject->createMutableRootShaderObject(
        getInnerObj(program), outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    outObject->m_device = this;
    outObject->m_slangType = nullptr;
    outObject->m_rootComponentType = getDebugObj(program)->m_slangProgram;
    returnComPtr(outRootObject, outObject);
    return result;
}

Result DebugDevice::createShaderObjectFromTypeLayout(
    slang::TypeLayoutReflection* typeLayout, IShaderObject** outShaderObject)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto result = baseObject->createShaderObjectFromTypeLayout(typeLayout, outObject->baseObject.writeRef());
    auto type = typeLayout->getType();
    auto typeName = type->getName();
    outObject->m_typeName = typeName;
    outObject->m_device = this;
    outObject->m_slangType = type;
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outShaderObject, outObject);
    return result;
}

Result DebugDevice::createMutableShaderObjectFromTypeLayout(
    slang::TypeLayoutReflection* typeLayout, IShaderObject** outShaderObject)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto result = baseObject->createMutableShaderObjectFromTypeLayout(
        typeLayout, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    auto type = typeLayout->getType();
    auto typeName = type->getName();
    outObject->m_typeName = typeName;
    outObject->m_device = this;
    outObject->m_slangType = type;
    returnComPtr(outShaderObject, outObject);
    return result;
}

Result DebugDevice::createProgram(
    const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnostics)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderProgram> outObject = new DebugShaderProgram(desc);
    auto result = baseObject->createProgram(desc, outObject->baseObject.writeRef(), outDiagnostics);
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outProgram, outObject);
    return result;
}

Result DebugDevice::createGraphicsPipelineState(
    const GraphicsPipelineStateDesc& desc,
    IPipelineState** outState)
{
    SLANG_GFX_API_FUNC;

    GraphicsPipelineStateDesc innerDesc = desc;
    innerDesc.program =
        desc.program ? static_cast<DebugShaderProgram*>(desc.program)->baseObject : nullptr;
    innerDesc.inputLayout =
        desc.inputLayout ? static_cast<DebugInputLayout*>(desc.inputLayout)->baseObject : nullptr;
    innerDesc.framebufferLayout =
        desc.framebufferLayout
            ? static_cast<DebugFramebufferLayout*>(desc.framebufferLayout)->baseObject
            : nullptr;
    RefPtr<DebugPipelineState> outObject = new DebugPipelineState();
    auto result =
        baseObject->createGraphicsPipelineState(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outState, outObject);
    return result;
}

Result DebugDevice::createComputePipelineState(
    const ComputePipelineStateDesc& desc,
    IPipelineState** outState)
{
    SLANG_GFX_API_FUNC;

    ComputePipelineStateDesc innerDesc = desc;
    innerDesc.program = static_cast<DebugShaderProgram*>(desc.program)->baseObject;

    RefPtr<DebugPipelineState> outObject = new DebugPipelineState();
    auto result =
        baseObject->createComputePipelineState(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outState, outObject);
    return result;
}

Result DebugDevice::createRayTracingPipelineState(
    const RayTracingPipelineStateDesc& desc,
    IPipelineState** outState)
{
    SLANG_GFX_API_FUNC;

    RayTracingPipelineStateDesc innerDesc = desc;
    innerDesc.program = static_cast<DebugShaderProgram*>(desc.program)->baseObject;

    RefPtr<DebugPipelineState> outObject = new DebugPipelineState();
    auto result =
        baseObject->createRayTracingPipelineState(innerDesc, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outState, outObject);
    return result;
}

SlangResult DebugDevice::readTextureResource(
    ITextureResource* resource,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    SLANG_GFX_API_FUNC;
    auto resourceImpl = static_cast<DebugTextureResource*>(resource);
    return baseObject->readTextureResource(
        resourceImpl->baseObject, state, outBlob, outRowPitch, outPixelSize);
}

SlangResult DebugDevice::readBufferResource(
    IBufferResource* buffer,
    size_t offset,
    size_t size,
    ISlangBlob** outBlob)
{
    SLANG_GFX_API_FUNC;
    auto bufferImpl = static_cast<DebugBufferResource*>(buffer);
    return baseObject->readBufferResource(bufferImpl->baseObject, offset, size, outBlob);
}

const DeviceInfo& DebugDevice::getDeviceInfo() const
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDeviceInfo();
}

Result DebugDevice::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outPool)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugQueryPool> result = new DebugQueryPool();
    result->desc = desc;
    SLANG_RETURN_ON_FAIL(baseObject->createQueryPool(desc, result->baseObject.writeRef()));
    returnComPtr(outPool, result);
    return SLANG_OK;
}

Result DebugDevice::createFence(const IFence::Desc& desc, IFence** outFence)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugFence> result = new DebugFence();
    SLANG_RETURN_ON_FAIL(baseObject->createFence(desc, result->baseObject.writeRef()));
    returnComPtr(outFence, result);
    return SLANG_OK;
}

Result DebugDevice::waitForFences(
    uint32_t fenceCount, IFence** fences, uint64_t* values , bool waitForAll, uint64_t timeout)
{
    SLANG_GFX_API_FUNC;
    return baseObject->waitForFences(fenceCount, fences, values, waitForAll, timeout);
}

Result DebugDevice::getTextureAllocationInfo(
    const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getTextureAllocationInfo(desc, outSize, outAlignment);
}

Result DebugDevice::getTextureRowAlignment(size_t* outAlignment)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getTextureRowAlignment(outAlignment);
}

Result DebugDevice::createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outTable)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugShaderTable> result = new DebugShaderTable();
    SLANG_RETURN_ON_FAIL(baseObject->createShaderTable(desc, result->baseObject.writeRef()));
    returnComPtr(outTable, result);
    return SLANG_OK;
}

IResource::Type DebugBufferResource::getType()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getType();
}

IBufferResource::Desc* DebugBufferResource::getDesc()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDesc();
}

DeviceAddress DebugBufferResource::getDeviceAddress()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDeviceAddress();
}

Result DebugBufferResource::getNativeResourceHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeResourceHandle(outHandle);
}

Result DebugBufferResource::getSharedHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getSharedHandle(outHandle);
}

Result DebugBufferResource::setDebugName(const char* name)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setDebugName(name);
}

const char* DebugBufferResource::getDebugName()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDebugName();
}

Result DebugBufferResource::map(MemoryRange* rangeToRead, void** outPointer)
{
    SLANG_GFX_API_FUNC;
    return baseObject->map(rangeToRead, outPointer);
}

Result DebugBufferResource::unmap(MemoryRange* writtenRange)
{
    return baseObject->unmap(writtenRange);
}

IResource::Type DebugTextureResource::getType()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getType();
}

ITextureResource::Desc* DebugTextureResource::getDesc()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDesc();
}

Result DebugTextureResource::getNativeResourceHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeResourceHandle(outHandle);
}

Result DebugTextureResource::getSharedHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getSharedHandle(outHandle);
}

Result DebugTextureResource::setDebugName(const char* name)
{
    return baseObject->setDebugName(name);
}

const char* DebugTextureResource::getDebugName() { return baseObject->getDebugName(); }


DebugCommandBuffer::DebugCommandBuffer()
{
    SLANG_GFX_API_FUNC;
    m_renderCommandEncoder.commandBuffer = this;
    m_computeCommandEncoder.commandBuffer = this;
    m_resourceCommandEncoder.commandBuffer = this;
    m_rayTracingCommandEncoder.commandBuffer = this;
}

void DebugCommandBuffer::encodeRenderCommands(
    IRenderPassLayout* renderPass,
    IFramebuffer* framebuffer,
    IRenderCommandEncoder** outEncoder)
{
    SLANG_GFX_API_FUNC;
    checkCommandBufferOpenWhenCreatingEncoder();
    checkEncodersClosedBeforeNewEncoder();
    auto innerRenderPass =
        renderPass ? static_cast<DebugRenderPassLayout*>(renderPass)->baseObject : nullptr;
    auto innerFramebuffer =
        framebuffer ? static_cast<DebugFramebuffer*>(framebuffer)->baseObject : nullptr;
    m_renderCommandEncoder.isOpen = true;
    baseObject->encodeRenderCommands(
        innerRenderPass, innerFramebuffer, &m_renderCommandEncoder.baseObject);
    if (m_renderCommandEncoder.baseObject)
        *outEncoder = &m_renderCommandEncoder;
    else
        *outEncoder = nullptr;
}

void DebugCommandBuffer::encodeComputeCommands(IComputeCommandEncoder** outEncoder)
{
    SLANG_GFX_API_FUNC;
    checkCommandBufferOpenWhenCreatingEncoder();
    checkEncodersClosedBeforeNewEncoder();
    m_computeCommandEncoder.isOpen = true;
    baseObject->encodeComputeCommands(&m_computeCommandEncoder.baseObject);
    if (m_computeCommandEncoder.baseObject)
    {
        *outEncoder = &m_computeCommandEncoder;
    }
    else
    {
        *outEncoder = nullptr;
    }
}

void DebugCommandBuffer::encodeResourceCommands(IResourceCommandEncoder** outEncoder)
{
    SLANG_GFX_API_FUNC;
    checkCommandBufferOpenWhenCreatingEncoder();
    checkEncodersClosedBeforeNewEncoder();
    m_resourceCommandEncoder.isOpen = true;
    baseObject->encodeResourceCommands(&m_resourceCommandEncoder.baseObject);
    if (m_resourceCommandEncoder.baseObject)
    {
        *outEncoder = &m_resourceCommandEncoder;
    }
    else
    {
        *outEncoder = nullptr;
    }
}

void DebugCommandBuffer::encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder)
{
    SLANG_GFX_API_FUNC;
    checkCommandBufferOpenWhenCreatingEncoder();
    checkEncodersClosedBeforeNewEncoder();
    m_rayTracingCommandEncoder.isOpen = true;
    baseObject->encodeRayTracingCommands(&m_rayTracingCommandEncoder.baseObject);
    if (m_rayTracingCommandEncoder.baseObject)
    {
        *outEncoder = &m_rayTracingCommandEncoder;
    }
    else
    {
        *outEncoder = nullptr;
    }
}

void DebugCommandBuffer::close()
{
    SLANG_GFX_API_FUNC;
    if (!isOpen)
    {
        GFX_DIAGNOSE_ERROR("command buffer is already closed.");
    }
    if (m_renderCommandEncoder.isOpen)
    {
        GFX_DIAGNOSE_ERROR(
            "A render command encoder on this command buffer is still open. "
            "IRenderCommandEncoder::endEncoding() must be called before closing a command buffer.");
    }
    if (m_computeCommandEncoder.isOpen)
    {
        GFX_DIAGNOSE_ERROR(
            "A compute command encoder on this command buffer is still open. "
            "IComputeCommandEncoder::endEncoding() must be called before closing a command buffer.");
    }
    if (m_resourceCommandEncoder.isOpen)
    {
        GFX_DIAGNOSE_ERROR(
            "A resource command encoder on this command buffer is still open. "
            "IResourceCommandEncoder::endEncoding() must be called before closing a command buffer.");
    }
    isOpen = false;
    baseObject->close();
}

Result DebugCommandBuffer::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeHandle(outHandle);
}

void DebugCommandBuffer::checkEncodersClosedBeforeNewEncoder()
{
    if (m_renderCommandEncoder.isOpen || m_resourceCommandEncoder.isOpen ||
        m_computeCommandEncoder.isOpen)
    {
        GFX_DIAGNOSE_ERROR(
            "A previouse command encoder created on this command buffer is still open. "
            "endEncoding() must be called on the encoder before creating an encoder.");
    }
}

void DebugCommandBuffer::checkCommandBufferOpenWhenCreatingEncoder()
{
    if (!isOpen)
    {
        GFX_DIAGNOSE_ERROR("The command buffer is already closed. Encoders can only be retrieved "
                           "while the command buffer is open.");
    }
}

void DebugComputeCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    isOpen = false;
    baseObject->endEncoding();
}

Result DebugComputeCommandEncoder::bindPipeline(
    IPipelineState* state,
    IShaderObject** outRootShaderObject)
{
    SLANG_GFX_API_FUNC;

    auto innerState = static_cast<DebugPipelineState*>(state)->baseObject;
    IShaderObject* innerRootObject = nullptr;
    commandBuffer->rootObject.reset();
    auto result = baseObject->bindPipeline(innerState, &innerRootObject);
    commandBuffer->rootObject.baseObject.attach(innerRootObject);
    *outRootShaderObject = &commandBuffer->rootObject;
    return result;
}

Result DebugComputeCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* state, IShaderObject* rootObject)
{
    SLANG_GFX_API_FUNC;
    return baseObject->bindPipelineWithRootObject(getInnerObj(state), getInnerObj(rootObject));
}

void DebugComputeCommandEncoder::dispatchCompute(int x, int y, int z)
{
    SLANG_GFX_API_FUNC;
    baseObject->dispatchCompute(x, y, z);
}

void DebugComputeCommandEncoder::dispatchComputeIndirect(
    IBufferResource* cmdBuffer, uint64_t offset)
{
    SLANG_GFX_API_FUNC;
    baseObject->dispatchComputeIndirect(getInnerObj(cmdBuffer), offset);
}

void DebugRenderCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    isOpen = false;
    baseObject->endEncoding();
}

Result DebugRenderCommandEncoder::bindPipeline(
    IPipelineState* state,
    IShaderObject** outRootShaderObject)
{
    SLANG_GFX_API_FUNC;

    auto innerState = static_cast<DebugPipelineState*>(state)->baseObject;
    IShaderObject* innerRootObject = nullptr;
    commandBuffer->rootObject.reset();
    auto result = baseObject->bindPipeline(innerState, &innerRootObject);
    commandBuffer->rootObject.baseObject.attach(innerRootObject);
    *outRootShaderObject = &commandBuffer->rootObject;
    return result;
}

Result DebugRenderCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* state, IShaderObject* rootObject)
{
    SLANG_GFX_API_FUNC;
    return baseObject->bindPipelineWithRootObject(getInnerObj(state), getInnerObj(rootObject));
}

void DebugRenderCommandEncoder::setViewports(uint32_t count, const Viewport* viewports)
{
    SLANG_GFX_API_FUNC;
    baseObject->setViewports(count, viewports);
}

void DebugRenderCommandEncoder::setScissorRects(uint32_t count, const ScissorRect* scissors)
{
    SLANG_GFX_API_FUNC;
    baseObject->setScissorRects(count, scissors);
}

void DebugRenderCommandEncoder::setPrimitiveTopology(PrimitiveTopology topology)
{
    SLANG_GFX_API_FUNC;
    baseObject->setPrimitiveTopology(topology);
}

void DebugRenderCommandEncoder::setVertexBuffers(
    uint32_t startSlot,
    uint32_t slotCount,
    IBufferResource* const* buffers,
    const uint32_t* offsets)
{
    SLANG_GFX_API_FUNC;

    List<IBufferResource*> innerBuffers;
    for (UInt i = 0; i < slotCount; i++)
    {
        innerBuffers.add(static_cast<DebugBufferResource*>(buffers[i])->baseObject.get());
    }
    baseObject->setVertexBuffers(startSlot, slotCount, innerBuffers.getBuffer(), offsets);
}

void DebugRenderCommandEncoder::setIndexBuffer(
    IBufferResource* buffer, Format indexFormat, uint32_t offset)
{
    SLANG_GFX_API_FUNC;
    auto innerBuffer = static_cast<DebugBufferResource*>(buffer)->baseObject.get();
    baseObject->setIndexBuffer(innerBuffer, indexFormat, offset);
}

void DebugRenderCommandEncoder::draw(uint32_t vertexCount, uint32_t startVertex)
{
    SLANG_GFX_API_FUNC;
    baseObject->draw(vertexCount, startVertex);
}

void DebugRenderCommandEncoder::drawIndexed(
    uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
{
    SLANG_GFX_API_FUNC;
    baseObject->drawIndexed(indexCount, startIndex, baseVertex);
}

void DebugRenderCommandEncoder::drawIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    SLANG_GFX_API_FUNC;
    baseObject->drawIndirect(
        maxDrawCount, getInnerObj(argBuffer), argOffset, getInnerObj(countBuffer), countOffset);
}

void DebugRenderCommandEncoder::drawIndexedIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    SLANG_GFX_API_FUNC;
    baseObject->drawIndexedIndirect(
        maxDrawCount, getInnerObj(argBuffer), argOffset, getInnerObj(countBuffer), countOffset);
}

void DebugRenderCommandEncoder::setStencilReference(uint32_t referenceValue)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setStencilReference(referenceValue);
}

Result DebugRenderCommandEncoder::setSamplePositions(
    uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition* samplePositions)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setSamplePositions(samplesPerPixel, pixelCount, samplePositions);
}

void DebugRenderCommandEncoder::drawInstanced(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t startVertex,
    uint32_t startInstanceLocation)
{
    SLANG_GFX_API_FUNC;
    return baseObject->drawInstanced(
        vertexCount, instanceCount, startVertex, startInstanceLocation);
}

void DebugRenderCommandEncoder::drawIndexedInstanced(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t startIndexLocation,
    int32_t baseVertexLocation,
    uint32_t startInstanceLocation)
{
    SLANG_GFX_API_FUNC;
    return baseObject->drawIndexedInstanced(
        indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void DebugResourceCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    isOpen = false;
    baseObject->endEncoding();
}

void DebugResourceCommandEncoderImpl::writeTimestamp(IQueryPool* pool, SlangInt index)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->writeTimestamp(static_cast<DebugQueryPool*>(pool)->baseObject, index);
}

void DebugResourceCommandEncoderImpl::copyBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    IBufferResource* src,
    size_t srcOffset,
    size_t size)
{
    SLANG_GFX_API_FUNC;
    auto dstImpl = static_cast<DebugBufferResource*>(dst);
    auto srcImpl = static_cast<DebugBufferResource*>(src);
    getBaseResourceEncoder()->copyBuffer(
        dstImpl->baseObject, dstOffset, srcImpl->baseObject, srcOffset, size);
}

void DebugResourceCommandEncoderImpl::uploadBufferData(
    IBufferResource* dst,
    size_t offset,
    size_t size,
    void* data)
{
    SLANG_GFX_API_FUNC;
    auto dstImpl = static_cast<DebugBufferResource*>(dst);
    getBaseResourceEncoder()->uploadBufferData(dstImpl->baseObject, offset, size, data);
}

void DebugResourceCommandEncoderImpl::textureBarrier(
    size_t count,
    ITextureResource* const* textures,
    ResourceState src,
    ResourceState dst)
{
    SLANG_GFX_API_FUNC;

    List<ITextureResource*> innerTextures;
    for (size_t i = 0; i < count; i++)
    {
        innerTextures.add(static_cast<DebugTextureResource*>(textures[i])->baseObject.get());
    }
    getBaseResourceEncoder()->textureBarrier(count, innerTextures.getBuffer(), src, dst);
}

void DebugResourceCommandEncoderImpl::bufferBarrier(
    size_t count,
    IBufferResource* const* buffers,
    ResourceState src,
    ResourceState dst)
{
    SLANG_GFX_API_FUNC;

    List<IBufferResource*> innerBuffers;
    for(size_t i = 0; i < count; i++)
    {
        innerBuffers.add(static_cast<DebugBufferResource*>(buffers[i])->baseObject.get());
    }
    getBaseResourceEncoder()->bufferBarrier(count, innerBuffers.getBuffer(), src, dst);
}

void DebugResourceCommandEncoderImpl::copyTexture(
    ITextureResource* dst,
    ResourceState dstState,
    SubresourceRange dstSubresource,
    ITextureResource::Offset3D dstOffset,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Size extent)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->copyTexture(
        getInnerObj(dst),
        dstState,
        dstSubresource,
        dstOffset,
        getInnerObj(src),
        srcState,
        srcSubresource,
        srcOffset,
        extent);
}

void DebugResourceCommandEncoderImpl::uploadTextureData(
    ITextureResource* dst,
    SubresourceRange subResourceRange,
    ITextureResource::Offset3D offset,
    ITextureResource::Size extent,
    ITextureResource::SubresourceData* subResourceData,
    size_t subResourceDataCount)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->uploadTextureData(
        getInnerObj(dst), subResourceRange, offset, extent, subResourceData, subResourceDataCount);
}

void DebugResourceCommandEncoderImpl::clearResourceView(
    IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags)
{
    SLANG_GFX_API_FUNC;
    switch (view->getViewDesc()->type)
    {
    case IResourceView::Type::DepthStencil:
    case IResourceView::Type::RenderTarget:
    case IResourceView::Type::UnorderedAccess:
        break;
    default:
        GFX_DIAGNOSE_ERROR_FORMAT(
            "Resource view %lld cannot be cleared. Only DepthStencil, "
            "RenderTarget or UnorderedAccess views can be cleared.",
            getDebugObj(view)->uid);
    }
    getBaseResourceEncoder()->clearResourceView(getInnerObj(view), clearValue, flags);
}

void DebugResourceCommandEncoderImpl::resolveResource(
    ITextureResource* source,
    ResourceState sourceState,
    SubresourceRange sourceRange,
    ITextureResource* dest,
    ResourceState destState,
    SubresourceRange destRange)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->resolveResource(
        getInnerObj(source), sourceState, sourceRange, getInnerObj(dest), destState, destRange);
}

void DebugResourceCommandEncoderImpl::resolveQuery(
    IQueryPool* queryPool, uint32_t index, uint32_t count, IBufferResource* buffer, uint64_t offset)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->resolveQuery(getInnerObj(queryPool), index, count, buffer, offset);
}

void DebugResourceCommandEncoderImpl::copyTextureToBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    size_t dstSize,
    size_t dstRowStride,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Size extent)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->copyTextureToBuffer(
        getInnerObj(dst), dstOffset, dstSize, dstRowStride, getInnerObj(src), srcState, srcSubresource, srcOffset, extent);
}

void DebugResourceCommandEncoderImpl::textureSubresourceBarrier(
    ITextureResource* texture,
    SubresourceRange subresourceRange,
    ResourceState src,
    ResourceState dst)
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->textureSubresourceBarrier(
        getInnerObj(texture), subresourceRange, src, dst);
}

void DebugResourceCommandEncoderImpl::beginDebugEvent(const char* name, float rgbColor[3])
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->beginDebugEvent(name, rgbColor);
}

void DebugResourceCommandEncoderImpl::endDebugEvent()
{
    SLANG_GFX_API_FUNC;
    getBaseResourceEncoder()->endDebugEvent();
}

void DebugRayTracingCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    isOpen = false;
    baseObject->endEncoding();
}

void DebugRayTracingCommandEncoder::buildAccelerationStructure(
    const IAccelerationStructure::BuildDesc& desc,
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    SLANG_GFX_API_FUNC;
    IAccelerationStructure::BuildDesc innerDesc = desc;
    innerDesc.dest = getInnerObj(innerDesc.dest);
    innerDesc.source = getInnerObj(innerDesc.source);
    List<AccelerationStructureQueryDesc> innerQueryDescs;
    innerQueryDescs.addRange(queryDescs, propertyQueryCount);
    for (auto& innerQueryDesc : innerQueryDescs)
    {
        innerQueryDesc.queryPool = getInnerObj(innerQueryDesc.queryPool);
    }
    validateAccelerationStructureBuildInputs(desc.inputs);
    baseObject->buildAccelerationStructure(
        innerDesc, propertyQueryCount, innerQueryDescs.getBuffer());
}

void DebugRayTracingCommandEncoder::copyAccelerationStructure(
    IAccelerationStructure* dest,
    IAccelerationStructure* src,
    AccelerationStructureCopyMode mode)
{
    SLANG_GFX_API_FUNC;
    auto innerDest = getInnerObj(dest);
    auto innerSrc = getInnerObj(src);
    baseObject->copyAccelerationStructure(innerDest, innerSrc, mode);
}

void DebugRayTracingCommandEncoder::queryAccelerationStructureProperties(
    int accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    int queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    SLANG_GFX_API_FUNC;
    List<IAccelerationStructure*> innerAS;
    for (int i = 0; i < accelerationStructureCount; i++)
    {
        innerAS.add(getInnerObj(accelerationStructures[i]));
    }
    List<AccelerationStructureQueryDesc> innerQueryDescs;
    innerQueryDescs.addRange(queryDescs, queryCount);
    for (auto& innerQueryDesc : innerQueryDescs)
    {
        innerQueryDesc.queryPool = getInnerObj(innerQueryDesc.queryPool);
    }
    baseObject->queryAccelerationStructureProperties(
        accelerationStructureCount, innerAS.getBuffer(), queryCount, innerQueryDescs.getBuffer());
}

void DebugRayTracingCommandEncoder::serializeAccelerationStructure(
    DeviceAddress dest,
    IAccelerationStructure* source)
{
    SLANG_GFX_API_FUNC;
    baseObject->serializeAccelerationStructure(dest, getInnerObj(source));
}

void DebugRayTracingCommandEncoder::deserializeAccelerationStructure(
    IAccelerationStructure* dest,
    DeviceAddress source)
{
    SLANG_GFX_API_FUNC;
    baseObject->deserializeAccelerationStructure(getInnerObj(dest), source);
}

void DebugRayTracingCommandEncoder::bindPipeline(
    IPipelineState* state, IShaderObject** outRootObject)
{
    SLANG_GFX_API_FUNC;
    auto innerPipeline = getInnerObj(state);
    IShaderObject* innerRootObject = nullptr;
    commandBuffer->rootObject.reset();
    baseObject->bindPipeline(innerPipeline, &innerRootObject);
    commandBuffer->rootObject.baseObject.attach(innerRootObject);
    *outRootObject = &commandBuffer->rootObject;
}

Result DebugRayTracingCommandEncoder::bindPipelineWithRootObject(
    IPipelineState* state, IShaderObject* rootObject)
{
    SLANG_GFX_API_FUNC;
    return baseObject->bindPipelineWithRootObject(getInnerObj(state), getInnerObj(rootObject));
}

void DebugRayTracingCommandEncoder::dispatchRays(
    uint32_t rayGenShaderIndex,
    IShaderTable* shaderTable,
    int32_t width,
    int32_t height,
    int32_t depth)
{
    SLANG_GFX_API_FUNC;
    baseObject->dispatchRays(rayGenShaderIndex, getInnerObj(shaderTable), width, height, depth);
}

const ICommandQueue::Desc& DebugCommandQueue::getDesc()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDesc();
}

void DebugCommandQueue::executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    SLANG_GFX_API_FUNC;
    List<ICommandBuffer*> innerCommandBuffers;
    for (uint32_t i = 0; i < count; i++)
    {
        auto cmdBufferIn = commandBuffers[i];
        auto cmdBufferImpl = static_cast<DebugCommandBuffer*>(cmdBufferIn);
        auto innerCmdBuffer = cmdBufferImpl->baseObject.get();
        innerCommandBuffers.add(innerCmdBuffer);
        if (cmdBufferImpl->isOpen)
        {
            GFX_DIAGNOSE_ERROR_FORMAT(
                "Command buffer %lld is still open. A command buffer must be closed "
                "before submitting to a command queue.",
                cmdBufferImpl->uid);
        }
        if (i > 0)
        {
            if (cmdBufferImpl->m_transientHeap != getDebugObj(commandBuffers[0])->m_transientHeap)
            {
                GFX_DIAGNOSE_ERROR("Command buffers passed to a single executeCommandBuffers "
                                   "call must be allocated from the same transient heap.");
            }
        }
    }
    baseObject->executeCommandBuffers(count, innerCommandBuffers.getBuffer(), getInnerObj(fence), valueToSignal);
}

void DebugCommandQueue::waitOnHost()
{
    SLANG_GFX_API_FUNC;
    baseObject->waitOnHost();
}

Result DebugCommandQueue::waitForFenceValuesOnDevice(
    uint32_t fenceCount, IFence** fences, uint64_t* waitValues)
{
    SLANG_GFX_API_FUNC;
    List<IFence*> innerFences;
    for (uint32_t i = 0; i < fenceCount; ++i)
    {
        innerFences.add(getInnerObj(fences[i]));
    }
    return baseObject->waitForFenceValuesOnDevice(fenceCount, innerFences.getBuffer(), waitValues);
}

Result DebugCommandQueue::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeHandle(outHandle);
}

Result DebugTransientResourceHeap::synchronizeAndReset()
{
    SLANG_GFX_API_FUNC;
    return baseObject->synchronizeAndReset();
}

Result DebugTransientResourceHeap::finish()
{
    SLANG_GFX_API_FUNC;
    return baseObject->finish();
}

Result DebugTransientResourceHeap::createCommandBuffer(ICommandBuffer** outCommandBuffer)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugCommandBuffer> outObject = new DebugCommandBuffer();
    outObject->m_transientHeap = this;
    auto result = baseObject->createCommandBuffer(outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outCommandBuffer, outObject);
    return result;
}

const ISwapchain::Desc& DebugSwapchain::getDesc()
{
    SLANG_GFX_API_FUNC;
    desc = baseObject->getDesc();
    desc.queue = queue.Ptr();
    return desc;
}

Result DebugSwapchain::getImage(uint32_t index, ITextureResource** outResource)
{
    SLANG_GFX_API_FUNC;
    maybeRebuildImageList();
    if (index > (uint32_t)m_images.getCount())
    {
        GFX_DIAGNOSE_ERROR_FORMAT(
            "`index`(%d) must not exceed total number of images (%d) in the swapchain.",
            index,
            (uint32_t)m_images.getCount());
    }
    returnComPtr(outResource, m_images[index]);
    return SLANG_OK;
}

Result DebugSwapchain::present()
{
    SLANG_GFX_API_FUNC;
    return baseObject->present();
}

int DebugSwapchain::acquireNextImage()
{
    SLANG_GFX_API_FUNC;
    return baseObject->acquireNextImage();
}

Result DebugSwapchain::resize(uint32_t width, uint32_t height)
{
    SLANG_GFX_API_FUNC;
    for (auto& image : m_images)
    {
        if (image->debugGetReferenceCount() != 1)
        {
            GFX_DIAGNOSE_ERROR("all swapchain images must be released before calling resize().");
            return SLANG_FAIL;
        }
    }
    m_images.clearAndDeallocate();
    return baseObject->resize(width, height);
}

bool DebugSwapchain::isOccluded()
{
    SLANG_GFX_API_FUNC;
    return baseObject->isOccluded();
}

Result DebugSwapchain::setFullScreenMode(bool mode)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setFullScreenMode(mode);
}

void DebugSwapchain::maybeRebuildImageList()
{
    SLANG_GFX_API_FUNC;
    if (m_images.getCount() != 0)
        return;
    m_images.clearAndDeallocate();
    for (uint32_t i = 0; i < baseObject->getDesc().imageCount; i++)
    {
        RefPtr<DebugTextureResource> image = new DebugTextureResource();
        baseObject->getImage(i, image->baseObject.writeRef());
        m_images.add(image);
    }
}

ShaderObjectContainerType DebugShaderObject::getContainerType()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getContainerType();
}

slang::TypeLayoutReflection* DebugShaderObject::getElementTypeLayout()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getElementTypeLayout();
}

UInt DebugShaderObject::getEntryPointCount()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getEntryPointCount();
}

Result DebugShaderObject::getEntryPoint(UInt index, IShaderObject** entryPoint)
{
    SLANG_GFX_API_FUNC;
    if (m_entryPoints.getCount() == 0)
    {
        for (UInt i = 0; i < getEntryPointCount(); i++)
        {
            RefPtr<DebugShaderObject> entryPointObj = new DebugShaderObject();
            SLANG_RETURN_ON_FAIL(
                baseObject->getEntryPoint(i, entryPointObj->baseObject.writeRef()));
            m_entryPoints.add(entryPointObj);
        }
    }
    if (index > (UInt)m_entryPoints.getCount())
    {
        GFX_DIAGNOSE_ERROR("`index` must not exceed `entryPointCount`.");
        return SLANG_FAIL;
    }
    returnComPtr(entryPoint, m_entryPoints[index]);
    return SLANG_OK;
}

Result DebugShaderObject::setData(ShaderOffset const& offset, void const* data, size_t size)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setData(offset, data, size);
}

Result DebugShaderObject::getObject(ShaderOffset const& offset, IShaderObject** object)
{
    SLANG_GFX_API_FUNC;

    ComPtr<IShaderObject> innerObject;
    auto resultCode = baseObject->getObject(offset, innerObject.writeRef());
    SLANG_RETURN_ON_FAIL(resultCode);
    RefPtr<DebugShaderObject> debugShaderObject;
    if (m_objects.TryGetValue(ShaderOffsetKey{offset}, debugShaderObject))
    {
        if (debugShaderObject->baseObject == innerObject)
        {
            returnComPtr(object, debugShaderObject);
            return resultCode;
        }
    }
    debugShaderObject = new DebugShaderObject();
    debugShaderObject->baseObject = innerObject;
    debugShaderObject->m_typeName = innerObject->getElementTypeLayout()->getName();
    m_objects[ShaderOffsetKey{offset}] = debugShaderObject;
    returnComPtr(object, debugShaderObject);
    return resultCode;
}

Result DebugShaderObject::setObject(ShaderOffset const& offset, IShaderObject* object)
{
    SLANG_GFX_API_FUNC;
    auto objectImpl = static_cast<DebugShaderObject*>(object);
    m_objects[ShaderOffsetKey{offset}] = objectImpl;
    return baseObject->setObject(offset, objectImpl->baseObject.get());
}

Result DebugShaderObject::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    SLANG_GFX_API_FUNC;
    auto viewImpl = static_cast<DebugResourceView*>(resourceView);
    m_resources[ShaderOffsetKey{offset}] = viewImpl;
    return baseObject->setResource(offset, viewImpl->baseObject.get());
}

Result DebugShaderObject::setSampler(ShaderOffset const& offset, ISamplerState* sampler)
{
    SLANG_GFX_API_FUNC;
    auto samplerImpl = static_cast<DebugSamplerState*>(sampler);
    m_samplers[ShaderOffsetKey{offset}] = samplerImpl;
    return baseObject->setSampler(offset, samplerImpl->baseObject.get());
}

Result DebugShaderObject::setCombinedTextureSampler(
    ShaderOffset const& offset,
    IResourceView* textureView,
    ISamplerState* sampler)
{
    SLANG_GFX_API_FUNC;
    auto samplerImpl = static_cast<DebugSamplerState*>(sampler);
    m_samplers[ShaderOffsetKey{offset}] = samplerImpl;
    auto viewImpl = static_cast<DebugResourceView*>(textureView);
    m_resources[ShaderOffsetKey{offset}] = viewImpl;
    return baseObject->setCombinedTextureSampler(
        offset, viewImpl->baseObject.get(), samplerImpl->baseObject.get());
}

Result DebugShaderObject::setSpecializationArgs(
    ShaderOffset const& offset,
    const slang::SpecializationArg* args,
    uint32_t count)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setSpecializationArgs(offset, args, count);
}

Result DebugShaderObject::getCurrentVersion(
    ITransientResourceHeap* transientHeap, IShaderObject** outObject)
{
    SLANG_GFX_API_FUNC;
    ComPtr<IShaderObject> innerObject;
    SLANG_RETURN_ON_FAIL(baseObject->getCurrentVersion(getInnerObj(transientHeap), innerObject.writeRef()));
    RefPtr<DebugShaderObject> debugShaderObject = new DebugShaderObject();
    debugShaderObject->baseObject = innerObject;
    debugShaderObject->m_typeName = innerObject->getElementTypeLayout()->getName();
    returnComPtr(outObject, debugShaderObject);
    return SLANG_OK;
}

const void* DebugShaderObject::getRawData()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getRawData();
}

size_t DebugShaderObject::getSize()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getSize();
}

Result DebugShaderObject::setConstantBufferOverride(IBufferResource* constantBuffer)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setConstantBufferOverride(getInnerObj(constantBuffer));
}

DebugObjectBase::DebugObjectBase()
{
    static uint64_t uidCounter = 0;
    uid = ++uidCounter;
}

Result DebugRootShaderObject::setSpecializationArgs(
    ShaderOffset const& offset,
    const slang::SpecializationArg* args,
    uint32_t count)
{
    SLANG_GFX_API_FUNC;

    return baseObject->setSpecializationArgs(offset, args, count);
}

void DebugRootShaderObject::reset()
{
    m_entryPoints.clear();
    m_objects.Clear();
    m_resources.Clear();
    m_samplers.Clear();
    baseObject.detach();
}

Result DebugQueryPool::getResult(SlangInt index, SlangInt count, uint64_t* data)
{
    SLANG_GFX_API_FUNC;

    if (index < 0 || index + count > desc.count)
        GFX_DIAGNOSE_ERROR("index is out of bounds.");
    return baseObject->getResult(index, count, data);
}

Result DebugQueryPool::reset()
{
    SLANG_GFX_API_FUNC;
    return baseObject->reset();
}

DeviceAddress DebugAccelerationStructure::getDeviceAddress()
{
    SLANG_GFX_API_FUNC;

    return baseObject->getDeviceAddress();
}

Result DebugAccelerationStructure::getNativeHandle(InteropHandle* outNativeHandle)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getNativeHandle(outNativeHandle);
}

IResourceView::Desc* DebugResourceView::getViewDesc()
{
    SLANG_GFX_API_FUNC;

    return baseObject->getViewDesc();
}

Result DebugResourceView::getNativeHandle(InteropHandle* outNativeHandle)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getNativeHandle(outNativeHandle);
}

Result DebugSamplerState::getNativeHandle(InteropHandle* outNativeHandle)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getNativeHandle(outNativeHandle);
}

IResourceView::Desc* DebugAccelerationStructure::getViewDesc()
{
    SLANG_GFX_API_FUNC;

    return baseObject->getViewDesc();
}

Result DebugFence::getSharedHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getSharedHandle(outHandle);
}

Result DebugFence::getNativeHandle(InteropHandle* outNativeHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeHandle(outNativeHandle);
}

Result DebugFence::getCurrentValue(uint64_t* outValue)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getCurrentValue(outValue);
}

Result DebugFence::setCurrentValue(uint64_t value)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setCurrentValue(value);
}

DebugShaderProgram::DebugShaderProgram(const IShaderProgram::Desc& desc)
{
    m_slangProgram = desc.slangGlobalScope;
}

Result DebugPipelineState::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_GFX_API_FUNC;
    return baseObject->getNativeHandle(outHandle);
}

} // namespace gfx
