// metal-device.cpp
#include "metal-device.h"

#include "metal-swap-chain.h"
#include "metal-util.h"
#include "../resource-desc-utils.h"
#include "metal-texture.h"
#include "metal-render-pass.h"
#include "metal-vertex-layout.h"
#include "metal-shader-program.h"
#include "metal-buffer.h"
//#include "metal-command-queue.h"
//#include "metal-fence.h"
//#include "metal-query.h"
//#include "metal-resource-views.h"
//#include "metal-sampler.h"
#include "metal-shader-object.h"
#include "metal-shader-object-layout.h"
//#include "metal-shader-table.h"
//#include "metal-transient-heap.h"
//#include "metal-pipeline-dump-layer.h"
//#include "metal-helper-functions.h"

#include "source/core/slang-platform.h"
namespace gfx
{

using namespace Slang;

namespace metal
{

static bool shouldDumpPipeline()
{
    StringBuilder dumpPipelineSettings;
    PlatformUtil::getEnvironmentVariable(toSlice("SLANG_GFX_DUMP_PIPELINE"), dumpPipelineSettings);
    return dumpPipelineSettings.produceString() == "1";
}

DeviceImpl::~DeviceImpl()
{
}

Result DeviceImpl::getNativeDeviceHandles(InteropHandles* outHandles)
{
    outHandles->handles[0].handleValue = reinterpret_cast<intptr_t>(m_device);
    outHandles->handles[0].api = InteropHandleAPI::Metal;
    return SLANG_OK;
}

SlangResult DeviceImpl::initialize(const Desc& desc)
{
    // Initialize device info.
    {
        m_info.apiName = "Metal";
        m_info.bindingStyle = BindingStyle::Metal;
        m_info.projectionStyle = ProjectionStyle::Metal;
        m_info.deviceType = DeviceType::Metal;
        m_info.adapterName = "default";
        static const float kIdentity[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    m_desc = desc;

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));
    SlangResult initDeviceResult = SLANG_OK;

    m_device = MTL::CreateSystemDefaultDevice();
    m_commandQueue = m_device->newCommandQueue();

    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        desc.extendedDescCount,
        desc.extendedDescs,
        SLANG_METAL,
        "sm_5_1",
        makeArray(slang::PreprocessorMacroDesc{ "__METAL__", "1" }).getView()));

    // TODO: expose via some other means
    if (captureEnabled())
    {
        MTL::CaptureManager* captureManager = MTL::CaptureManager::sharedCaptureManager();
        MTL::CaptureDescriptor* d = MTL::CaptureDescriptor::alloc()->init();
        MTL::CaptureDestination captureDest = MTL::CaptureDestination::CaptureDestinationGPUTraceDocument;
        if (!captureManager->supportsDestination(MTL::CaptureDestinationGPUTraceDocument))
        {
            std::cout << "Cannot capture MTL calls to document; ensure that Info.plist exists with 'MetalCaptureEnabled' set to 'true'." << std::endl;
            exit(1);
        }
        d->setDestination(MTL::CaptureDestinationGPUTraceDocument);
        d->setCaptureObject(m_device);
        std::string cpath("frame.gputrace");
        NS::String* path = NS::String::alloc()->init(cpath.c_str(), NS::UTF8StringEncoding);
        NS::URL* url = NS::URL::alloc()->initFileURLWithPath(path);
        d->setOutputURL(url);
        NS::Error* errorCode = NS::Error::alloc();
        if (!captureManager->startCapture(d, &errorCode))
        {
            NS::String* errorString = errorCode->description();
            std::string estr(errorString->cString(NS::UTF8StringEncoding));
            std::cout << "Start capture failure: " << estr << std::endl;
            exit(1);
        }
    }
    return SLANG_OK;
}

//void DeviceImpl::waitForGpu() { m_deviceQueue.flushAndWait(); }


SLANG_NO_THROW const DeviceInfo& SLANG_MCALL DeviceImpl::getDeviceInfo() const { return m_info; }

Result DeviceImpl::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    returnComPtr(outHeap, result);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    if (m_queueAllocCount != 0)
        return SLANG_FAIL;
    RefPtr<CommandQueueImpl> result = new CommandQueueImpl;
    result->init(this);
    returnComPtr(outQueue, result);
    m_queueAllocCount++;
    return SLANG_OK;
}

Result DeviceImpl::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> sc = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(sc->init(this, desc, window));
    returnComPtr(outSwapchain, sc);
    return SLANG_OK;
}

Result DeviceImpl::createFramebufferLayout(
    const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl;
    SLANG_RETURN_ON_FAIL(layout->init(this, desc));
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl;
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result DeviceImpl::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> fb = new FramebufferImpl;
    SLANG_RETURN_ON_FAIL(fb->init(this, desc));
    returnComPtr(outFramebuffer, fb);
    return SLANG_OK;
}

SlangResult DeviceImpl::readTextureResource(
    ITextureResource* texture,
    ResourceState state,
    ISlangBlob** outBlob,
    Size* outRowPitch,
    Size* outPixelSize)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult DeviceImpl::readBufferResource(
    IBufferResource* inBuffer, Offset offset, Size size, ISlangBlob** outBlob)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outAS)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::getTextureAllocationInfo(
    const ITextureResource::Desc& descIn, Size* outSize, Size* outAlignment)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::getTextureRowAlignment(Size* outAlignment)
{
    *outAlignment = 1;
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createTextureResource(
    const ITextureResource::Desc& descIn,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
{
    TextureResource::Desc desc = fixupTextureDesc(descIn);

    const MTL::PixelFormat format = MetalUtil::getMetalPixelFormat(desc.format);
    if (format == MTL::PixelFormat::PixelFormatInvalid)
    {
        assert(!"Unsupported texture format");
        return SLANG_FAIL;
    }

    RefPtr<TextureResourceImpl> textureResource(new TextureResourceImpl(desc, this));
    //textureResource->m_metalFormat = format;
    MTL::TextureDescriptor* metalDesc = MTL::TextureDescriptor::alloc()->init();
    metalDesc->setStorageMode(MTL::StorageMode::StorageModePrivate);
    // Create the texture
    switch (desc.type)
    {
    case IResource::Type::Texture1D:
    {
        metalDesc->setTextureType(MTL::TextureType::TextureType1D);
        metalDesc->setWidth(descIn.size.width);
        break;
    }
    case IResource::Type::Texture2D:
    {
        metalDesc->setTextureType(MTL::TextureType::TextureType2D);
        metalDesc->setWidth(descIn.size.width);
        metalDesc->setHeight(descIn.size.height);
        break;
    }
    case IResource::Type::TextureCube:
    {
        metalDesc->setTextureType(MTL::TextureType::TextureTypeCube);
        metalDesc->setWidth(descIn.size.width);
        metalDesc->setHeight(descIn.size.height);
        break;
    }
    case IResource::Type::Texture3D:
    {
        metalDesc->setTextureType(MTL::TextureType::TextureType3D);
        metalDesc->setWidth(descIn.size.width);
        metalDesc->setHeight(descIn.size.height);
        metalDesc->setDepth(descIn.size.depth);
        break;
    }
    default:
    {
        assert("!Unsupported texture type");
        return SLANG_FAIL;
    }
    }
    metalDesc->setMipmapLevelCount(desc.numMipLevels);
    const int arraySize(calcEffectiveArraySize(desc));
    metalDesc->setArrayLength(arraySize);
    metalDesc->setPixelFormat(format);
    //metalDesc.setResourceOptions();
    metalDesc->setUsage(MTL::TextureUsageUnknown);
    metalDesc->setSampleCount(desc.sampleDesc.numSamples);
    textureResource->m_texture = m_device->newTexture(metalDesc);

    returnComPtr(outResource, textureResource);
    return SLANG_OK;
}

Result DeviceImpl::createBufferResource(
    const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc desc = fixupBufferDesc(descIn);

    const Size bufferSize = desc.sizeInBytes;

    MTL::ResourceOptions opts = (desc.memoryType == MemoryType::DeviceLocal ? MTL::ResourceStorageModePrivate : 0);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(desc, this));

    if (initData)
    {
        buffer->m_buffer = m_device->newBuffer(initData, bufferSize, opts);
    }
    else
    {
        buffer->m_buffer = m_device->newBuffer(bufferSize, opts);
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createBufferFromNativeHandle(
    InteropHandle handle, const IBufferResource::Desc& srcDesc, IBufferResource** outResource)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createTextureView(
    ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = static_cast<TextureResourceImpl*>(texture);
    RefPtr<TextureResourceViewImpl> view = new TextureResourceViewImpl(this);
    view->m_desc = desc;
    view->m_device = this;
    if (texture == nullptr)
    {
        view->m_texture = nullptr;
        returnComPtr(outView, view);
        return SLANG_OK;
    }

    bool isArray = resourceImpl->getDesc()->arraySize > 1;
    MTL::PixelFormat pixelFormat = MetalUtil::getMetalPixelFormat(desc.format);
    NS::Range levelRange(desc.subresourceRange.baseArrayLayer, std::max(desc.subresourceRange.layerCount, 1));
    NS::Range sliceRange(desc.subresourceRange.mipLevel, std::max(desc.subresourceRange.mipLevelCount, 1));
    MTL::TextureType textureType;
    switch (resourceImpl->getType())
    {
    case IResource::Type::Texture1D:
        textureType = isArray ? MTL::TextureType1DArray : MTL::TextureType1D;
        break;
    case IResource::Type::Texture2D:
        textureType = isArray ? MTL::TextureType2DArray : MTL::TextureType2D;
        break;
    case IResource::Type::Texture3D:
    {
        if (isArray) SLANG_UNIMPLEMENTED_X("Metal does not support arrays of 3D textures.");
        textureType = MTL::TextureType3D;
        break;
    }
    case IResource::Type::TextureCube:
        textureType = isArray ? MTL::TextureTypeCube : MTL::TextureTypeCubeArray;
        break;
    default:
        SLANG_UNIMPLEMENTED_X("Unsupported texture type.");
        break;
    }
    ITextureResource::Desc newDesc = *texture->getDesc();
    newDesc.numMipLevels = levelRange.length;
    newDesc.arraySize = sliceRange.length;

    view->m_type = ResourceViewImpl::ViewType::Texture;
    view->m_texture = resourceImpl; //new TextureResourceImpl(newDesc, this);
    view->m_texture->m_isCurrentDrawable = resourceImpl->m_isCurrentDrawable;
    view->m_texture->m_texture = resourceImpl->m_texture->newTextureView(pixelFormat, textureType, levelRange, sliceRange);
    returnComPtr(outView, view);

    return SLANG_OK;
}

Result DeviceImpl::getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createBufferView(
    IBufferResource* buffer,
    IBufferResource* counterBuffer,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

static MTL::VertexStepFunction translateVertexStepFunction(const InputSlotClass& slotClass)
{
    switch (slotClass)
    {
    case InputSlotClass::PerInstance:       return MTL::VertexStepFunctionPerInstance;
    case InputSlotClass::PerVertex:
    default:                                return MTL::VertexStepFunctionPerVertex;
    }
}

Result DeviceImpl::createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);
    List<MTL::VertexDescriptor*>& dstAttributes = layout->m_vertexDescs;
    List<MTL::VertexBufferLayoutDescriptor*>& dstBufferLayouts = layout->m_bufferLayoutDescs;

    const InputElementDesc* srcElements = desc.inputElements;
    Int numElements = desc.inputElementCount;

    const VertexStreamDesc* srcVertexStreams = desc.vertexStreams;
    Int vertexStreamCount = desc.vertexStreamCount;

    dstAttributes.setCount(numElements);
    dstBufferLayouts.setCount(vertexStreamCount);

    for (Int i = 0; i < vertexStreamCount; ++i)
    {
        auto& vbld = dstBufferLayouts[i];
        auto& srcStream = srcVertexStreams[i];
        vbld->setStepFunction(translateVertexStepFunction(srcStream.slotClass));
        vbld->setStepRate(srcStream.instanceDataStepRate);
        vbld->setStride(srcStream.stride);
    }

    for (Int i = 0; i < numElements; ++i)
    {
        auto& srcAttrib = srcElements[i];
        auto& dstAttrib = dstAttributes[i];
        dstAttrib->attributes()->object(i)->setOffset(srcAttrib.offset);
        dstAttrib->attributes()->object(i)->setBufferIndex(srcAttrib.bufferSlotIndex);
        MTL::VertexFormat metalFormat = MetalUtil::getMetalVertexFormat(srcAttrib.format);
        if (metalFormat == MTL::VertexFormatInvalid)
        {
            return SLANG_FAIL;
        }
        dstAttrib->attributes()->object(i)->setFormat(metalFormat);
    }

    return SLANG_OK;
}

Result DeviceImpl::createProgram(
    const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnosticBlob)
{
    // TODO:
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(this);
    shaderProgram->init(desc);

    //m_deviceObjectsWithPotentialBackReferences.add(shaderProgram);

    RootShaderObjectLayout::create(
        this,
        shaderProgram->linkedProgram,
        shaderProgram->linkedProgram->getLayout(),
        shaderProgram->m_rootObjectLayout.writeRef());
    returnComPtr(outProgram, shaderProgram);

    return SLANG_OK;
}

Result DeviceImpl::createShaderObjectLayout(
    slang::ISession* session,
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createMutableShaderObject(
    ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createGraphicsPipelineState(
    const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    pipelineStateImpl->establishStrongDeviceReference();
    //m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createComputePipelineState(
    const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createRayTracingPipelineState(
    const RayTracingPipelineStateDesc& desc, IPipelineState** outState)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outPool)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::createFence(const IFence::Desc& desc, IFence** outFence)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

Result DeviceImpl::waitForFences(
    GfxCount fenceCount, IFence** fences, uint64_t* fenceValues, bool waitForAll, uint64_t timeout)
{
    return SLANG_E_NOT_IMPLEMENTED;
}

} // namespace metal
} // namespace gfx
