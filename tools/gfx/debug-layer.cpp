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
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(ComputeCommandEncoder, CommandEncoder)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(RenderCommandEncoder, CommandEncoder)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(ResourceCommandEncoder, CommandEncoder)
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

#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL
#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT

Result DebugDevice::getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount)
{
    SLANG_GFX_API_FUNC;

    return baseObject->getFeatures(outFeatures, bufferSize, outFeatureCount);
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

Result DebugDevice::createBufferResource(
    const IBufferResource::Desc& desc,
    const void* initData,
    IBufferResource** outResource)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugBufferResource> outObject = new DebugBufferResource();
    auto result =
        baseObject->createBufferResource(desc, initData, outObject->baseObject.writeRef());
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
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugResourceView> outObject = new DebugResourceView();
    auto result = baseObject->createBufferView(
        static_cast<DebugBufferResource*>(buffer)->baseObject,
        desc,
        outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outView, outObject);
    return result;
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
    const InputElementDesc* inputElements,
    UInt inputElementCount,
    IInputLayout** outLayout)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugInputLayout> outObject = new DebugInputLayout();
    auto result = baseObject->createInputLayout(
        inputElements, inputElementCount, outObject->baseObject.writeRef());
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

Result DebugDevice::createShaderObject(slang::TypeReflection* type, IShaderObject** outShaderObject)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderObject> outObject = new DebugShaderObject();
    auto result = baseObject->createShaderObject(type, outObject->baseObject.writeRef());
    if (SLANG_FAILED(result))
        return result;
    returnComPtr(outShaderObject, outObject);
    return result;
}

Result DebugDevice::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    SLANG_GFX_API_FUNC;

    RefPtr<DebugShaderProgram> outObject = new DebugShaderProgram();
    auto result = baseObject->createProgram(desc, outObject->baseObject.writeRef());
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

DebugCommandBuffer::DebugCommandBuffer()
{
    SLANG_GFX_API_FUNC;
    m_renderCommandEncoder.commandBuffer = this;
    m_computeCommandEncoder.commandBuffer = this;
    m_resourceCommandEncoder.commandBuffer = this;
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
        innerRenderPass, innerFramebuffer, m_renderCommandEncoder.baseObject.writeRef());
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
    baseObject->encodeComputeCommands(m_computeCommandEncoder.baseObject.writeRef());
    *outEncoder = &m_computeCommandEncoder;
}

void DebugCommandBuffer::encodeResourceCommands(IResourceCommandEncoder** outEncoder)
{
    SLANG_GFX_API_FUNC;
    checkCommandBufferOpenWhenCreatingEncoder();
    checkEncodersClosedBeforeNewEncoder();
    m_resourceCommandEncoder.isOpen = true;
    baseObject->encodeResourceCommands(m_resourceCommandEncoder.baseObject.writeRef());
    *outEncoder = &m_resourceCommandEncoder;
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
    baseObject->endEncoding();
}

Result DebugComputeCommandEncoder::bindPipeline(
    IPipelineState* state,
    IShaderObject** outRootShaderObject)
{
    SLANG_GFX_API_FUNC;

    auto innerState = static_cast<DebugPipelineState*>(state)->baseObject;
    auto result =
        baseObject->bindPipeline(innerState, commandBuffer->rootObject.baseObject.writeRef());
    *outRootShaderObject = &commandBuffer->rootObject;
    return result;
}

void DebugComputeCommandEncoder::dispatchCompute(int x, int y, int z)
{
    SLANG_GFX_API_FUNC;
    baseObject->dispatchCompute(x, y, z);
}

void DebugRenderCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    baseObject->endEncoding();
}

Result DebugRenderCommandEncoder::bindPipeline(
    IPipelineState* state,
    IShaderObject** outRootShaderObject)
{
    SLANG_GFX_API_FUNC;

    auto innerState = static_cast<DebugPipelineState*>(state)->baseObject;
    auto result =
        baseObject->bindPipeline(innerState, commandBuffer->rootObject.baseObject.writeRef());
    *outRootShaderObject = &commandBuffer->rootObject;
    return result;
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
    UInt startSlot,
    UInt slotCount,
    IBufferResource* const* buffers,
    const UInt* strides,
    const UInt* offsets)
{
    SLANG_GFX_API_FUNC;

    List<IBufferResource*> innerBuffers;
    for (UInt i = 0; i < slotCount; i++)
    {
        innerBuffers.add(static_cast<DebugBufferResource*>(buffers[i])->baseObject.get());
    }
    baseObject->setVertexBuffers(startSlot, slotCount, innerBuffers.getBuffer(), strides, offsets);
}

void DebugRenderCommandEncoder::setIndexBuffer(
    IBufferResource* buffer,
    Format indexFormat,
    UInt offset)
{
    SLANG_GFX_API_FUNC;
    auto innerBuffer = static_cast<DebugBufferResource*>(buffer)->baseObject.get();
    baseObject->setIndexBuffer(innerBuffer, indexFormat, offset);
}

void DebugRenderCommandEncoder::draw(UInt vertexCount, UInt startVertex)
{
    SLANG_GFX_API_FUNC;
    baseObject->draw(vertexCount, startVertex);
}

void DebugRenderCommandEncoder::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    SLANG_GFX_API_FUNC;
    baseObject->drawIndexed(indexCount, startIndex, baseVertex);
}

void DebugRenderCommandEncoder::setStencilReference(uint32_t referenceValue)
{
    SLANG_GFX_API_FUNC;
    return baseObject->setStencilReference(referenceValue);
}

void DebugResourceCommandEncoder::endEncoding()
{
    SLANG_GFX_API_FUNC;
    baseObject->endEncoding();
}

void DebugResourceCommandEncoder::copyBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    IBufferResource* src,
    size_t srcOffset,
    size_t size)
{
    SLANG_GFX_API_FUNC;
    auto dstImpl = static_cast<DebugBufferResource*>(dst);
    auto srcImpl = static_cast<DebugBufferResource*>(src);
    baseObject->copyBuffer(dstImpl->baseObject, dstOffset, srcImpl->baseObject, srcOffset, size);
}

void DebugResourceCommandEncoder::uploadBufferData(
    IBufferResource* dst,
    size_t offset,
    size_t size,
    void* data)
{
    SLANG_GFX_API_FUNC;
    auto dstImpl = static_cast<DebugBufferResource*>(dst);
    baseObject->uploadBufferData(dstImpl->baseObject, offset, size, data);
}

const ICommandQueue::Desc& DebugCommandQueue::getDesc()
{
    SLANG_GFX_API_FUNC;
    return baseObject->getDesc();
}

void DebugCommandQueue::executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers)
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
    }
    baseObject->executeCommandBuffers(count, innerCommandBuffers.getBuffer());
}

void DebugCommandQueue::wait() { baseObject->wait(); }

Result DebugTransientResourceHeap::synchronizeAndReset()
{
    SLANG_GFX_API_FUNC;
    return baseObject->synchronizeAndReset();
}

Result DebugTransientResourceHeap::createCommandBuffer(ICommandBuffer** outCommandBuffer)
{
    SLANG_GFX_API_FUNC;
    RefPtr<DebugCommandBuffer> outObject = new DebugCommandBuffer();
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

DebugObjectBase::DebugObjectBase()
{
    static uint64_t uidCounter = 0;
    uid = ++uidCounter;
}

} // namespace gfx
