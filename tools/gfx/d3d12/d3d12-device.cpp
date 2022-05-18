// d3d12-device.cpp
#include "d3d12-device.h"

#include "../nvapi/nvapi-util.h"
#include "d3d12-buffer.h"
#include "d3d12-fence.h"
#include "d3d12-framebuffer.h"
#include "d3d12-pipeline-state.h"
#include "d3d12-query.h"
#include "d3d12-render-pass.h"
#include "d3d12-resource-views.h"
#include "d3d12-sampler.h"
#include "d3d12-shader-object.h"
#include "d3d12-shader-program.h"
#include "d3d12-shader-table.h"
#include "d3d12-swap-chain.h"
#include "d3d12-vertex-layout.h"

#include "d3d12-helper-functions.h"

namespace gfx
{
namespace d3d12
{

using namespace Slang;

Result DeviceImpl::createBuffer(
    const D3D12_RESOURCE_DESC& resourceDesc,
    const void* srcData,
    Size srcDataSize,
    D3D12_RESOURCE_STATES finalState,
    D3D12Resource& resourceOut,
    bool isShared,
    MemoryType memoryType)
{
    const Size bufferSize = Size(resourceDesc.Width);

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
    if (isShared)
        flags |= D3D12_HEAP_FLAG_SHARED;

    D3D12_RESOURCE_DESC desc = resourceDesc;

    D3D12_RESOURCE_STATES initialState = finalState;

    switch (memoryType)
    {
    case MemoryType::ReadBack:
        assert(!srcData);

        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        initialState |= D3D12_RESOURCE_STATE_COPY_DEST;

        break;
    case MemoryType::Upload:

        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        initialState |= D3D12_RESOURCE_STATE_GENERIC_READ;

        break;
    case MemoryType::DeviceLocal:
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        initialState = (srcData ? D3D12_RESOURCE_STATE_COPY_DEST : finalState);
        break;
    default:
        return SLANG_FAIL;
    }

    // Create the resource.
    SLANG_RETURN_ON_FAIL(
        resourceOut.initCommitted(m_device, heapProps, flags, desc, initialState, nullptr));

    if (srcData)
    {
        D3D12Resource uploadResource;

        if (memoryType == MemoryType::DeviceLocal)
        {
            // If the buffer is on the default heap, create upload buffer.
            D3D12_RESOURCE_DESC uploadDesc(resourceDesc);
            uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            SLANG_RETURN_ON_FAIL(uploadResource.initCommitted(
                m_device,
                heapProps,
                D3D12_HEAP_FLAG_NONE,
                uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr));
        }

        // Be careful not to actually copy a resource here.
        D3D12Resource& uploadResourceRef =
            (memoryType == MemoryType::DeviceLocal) ? uploadResource : resourceOut;

        // Copy data to the intermediate upload heap and then schedule a copy
        // from the upload heap to the vertex buffer.
        UINT8* dstData;
        D3D12_RANGE readRange = {}; // We do not intend to read from this resource on the CPU.

        ID3D12Resource* dxUploadResource = uploadResourceRef.getResource();

        SLANG_RETURN_ON_FAIL(
            dxUploadResource->Map(0, &readRange, reinterpret_cast<void**>(&dstData)));
        ::memcpy(dstData, srcData, srcDataSize);
        dxUploadResource->Unmap(0, nullptr);

        if (memoryType == MemoryType::DeviceLocal)
        {
            auto encodeInfo = encodeResourceCommands();
            encodeInfo.d3dCommandList->CopyBufferRegion(
                resourceOut, 0, uploadResourceRef, 0, bufferSize);
            submitResourceCommandsAndWait(encodeInfo);
        }
    }

    return SLANG_OK;
}

Result DeviceImpl::captureTextureToSurface(
    TextureResourceImpl* resourceImpl,
    ResourceState state,
    ISlangBlob** outBlob,
    Size* outRowPitch,
    Size* outPixelSize)
{
    auto& resource = resourceImpl->m_resource;

    const D3D12_RESOURCE_STATES initialState = D3DUtil::getResourceState(state);

    const ITextureResource::Desc& gfxDesc = *resourceImpl->getDesc();
    const D3D12_RESOURCE_DESC desc = resource.getResource()->GetDesc();

    // Don't bother supporting MSAA for right now
    if (desc.SampleDesc.Count > 1)
    {
        fprintf(stderr, "ERROR: cannot capture multi-sample texture\n");
        return SLANG_FAIL;
    }

    FormatInfo formatInfo;
    gfxGetFormatInfo(gfxDesc.format, &formatInfo);
    Size bytesPerPixel = formatInfo.blockSizeInBytes / formatInfo.pixelsPerBlock;
    Size rowPitch = int(desc.Width) * bytesPerPixel;
    static const Size align = 256; // D3D requires minimum 256 byte alignment for texture data.
    rowPitch = (rowPitch + align - 1) & ~(align - 1); // Bit trick for rounding up
    Size bufferSize = rowPitch * int(desc.Height) * int(desc.DepthOrArraySize);
    if (outRowPitch)
        *outRowPitch = rowPitch;
    if (outPixelSize)
        *outPixelSize = bytesPerPixel;

    D3D12Resource stagingResource;
    {
        D3D12_RESOURCE_DESC stagingDesc;
        initBufferResourceDesc(bufferSize, stagingDesc);

        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        SLANG_RETURN_ON_FAIL(stagingResource.initCommitted(
            m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));
    }

    auto encodeInfo = encodeResourceCommands();
    auto currentState = D3DUtil::getResourceState(state);

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(currentState, D3D12_RESOURCE_STATE_COPY_SOURCE, submitter);
    }

    // Do the copy
    {
        D3D12_TEXTURE_COPY_LOCATION srcLoc;
        srcLoc.pResource = resource;
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dstLoc;
        dstLoc.pResource = stagingResource;
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstLoc.PlacedFootprint.Offset = 0;
        dstLoc.PlacedFootprint.Footprint.Format = desc.Format;
        dstLoc.PlacedFootprint.Footprint.Width = UINT(desc.Width);
        dstLoc.PlacedFootprint.Footprint.Height = UINT(desc.Height);
        dstLoc.PlacedFootprint.Footprint.Depth = UINT(desc.DepthOrArraySize);
        dstLoc.PlacedFootprint.Footprint.RowPitch = UINT(rowPitch);

        encodeInfo.d3dCommandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    }

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, currentState, submitter);
    }

    // Submit the copy, and wait for copy to complete
    submitResourceCommandsAndWait(encodeInfo);

    {
        ID3D12Resource* dxResource = stagingResource;

        UINT8* data;
        D3D12_RANGE readRange = { 0, bufferSize };

        SLANG_RETURN_ON_FAIL(dxResource->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        RefPtr<Slang::ListBlob> resultBlob = new Slang::ListBlob();
        resultBlob->m_data.setCount(bufferSize);
        memcpy(resultBlob->m_data.getBuffer(), data, bufferSize);
        dxResource->Unmap(0, nullptr);
        returnComPtr(outBlob, resultBlob);
        return SLANG_OK;
    }
}

Result DeviceImpl::getNativeDeviceHandles(InteropHandles* outHandles)
{
    outHandles->handles[0].handleValue = (uint64_t)m_device;
    outHandles->handles[0].api = InteropHandleAPI::D3D12;
    return SLANG_OK;
}

Result DeviceImpl::_createDevice(
    DeviceCheckFlags deviceCheckFlags,
    const UnownedStringSlice& nameMatch,
    D3D_FEATURE_LEVEL featureLevel,
    D3D12DeviceInfo& outDeviceInfo)
{
    if (m_dxDebug && (deviceCheckFlags & DeviceCheckFlag::UseDebug))
    {
        m_dxDebug->EnableDebugLayer();
    }

    outDeviceInfo.clear();

    ComPtr<IDXGIFactory> dxgiFactory;
    SLANG_RETURN_ON_FAIL(D3DUtil::createFactory(deviceCheckFlags, dxgiFactory));

    List<ComPtr<IDXGIAdapter>> dxgiAdapters;
    SLANG_RETURN_ON_FAIL(
        D3DUtil::findAdapters(deviceCheckFlags, nameMatch, dxgiFactory, dxgiAdapters));

    ComPtr<ID3D12Device> device;
    ComPtr<IDXGIAdapter> adapter;

    for (Index i = 0; i < dxgiAdapters.getCount(); ++i)
    {
        IDXGIAdapter* dxgiAdapter = dxgiAdapters[i];
        if (SLANG_SUCCEEDED(
            m_D3D12CreateDevice(dxgiAdapter, featureLevel, IID_PPV_ARGS(device.writeRef()))))
        {
            adapter = dxgiAdapter;
            break;
        }
    }

    if (!device)
    {
        return SLANG_FAIL;
    }

    if (m_dxDebug && (deviceCheckFlags & DeviceCheckFlag::UseDebug))
    {
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SLANG_SUCCEEDED(device->QueryInterface(infoQueue.writeRef())))
        {
            // Make break
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            if (m_extendedDesc.debugBreakOnD3D12Error)
            {
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            }
            D3D12_MESSAGE_ID hideMessages[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
            };
            D3D12_INFO_QUEUE_FILTER f = {};
            f.DenyList.NumIDs = (UINT)SLANG_COUNT_OF(hideMessages);
            f.DenyList.pIDList = hideMessages;
            infoQueue->AddStorageFilterEntries(&f);

            // Apparently there is a problem with sm 6.3 with spurious errors, with debug layer
            // enabled
            D3D12_FEATURE_DATA_SHADER_MODEL featureShaderModel;
            featureShaderModel.HighestShaderModel = D3D_SHADER_MODEL(0x63);
            SLANG_SUCCEEDED(device->CheckFeatureSupport(
                D3D12_FEATURE_SHADER_MODEL, &featureShaderModel, sizeof(featureShaderModel)));

            if (featureShaderModel.HighestShaderModel >= D3D_SHADER_MODEL(0x63))
            {
                // Filter out any messages that cause issues
                // TODO: Remove this when the debug layers work properly
                D3D12_MESSAGE_ID messageIds[] = {
                    // When the debug layer is enabled this error is triggered sometimes after a
                    // CopyDescriptorsSimple call The failed check validates that the source and
                    // destination ranges of the copy do not overlap. The check assumes descriptor
                    // handles are pointers to memory, but this is not always the case and the check
                    // fails (even though everything is okay).
                    D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,
                };

                // We filter INFO messages because they are way too many
                D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

                D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
                infoQueueFilter.DenyList.NumSeverities = SLANG_COUNT_OF(severities);
                infoQueueFilter.DenyList.pSeverityList = severities;
                infoQueueFilter.DenyList.NumIDs = SLANG_COUNT_OF(messageIds);
                infoQueueFilter.DenyList.pIDList = messageIds;

                infoQueue->PushStorageFilter(&infoQueueFilter);
            }
        }
    }

    // Get the descs
    {
        adapter->GetDesc(&outDeviceInfo.m_desc);

        // Look up GetDesc1 info
        ComPtr<IDXGIAdapter1> adapter1;
        if (SLANG_SUCCEEDED(adapter->QueryInterface(adapter1.writeRef())))
        {
            adapter1->GetDesc1(&outDeviceInfo.m_desc1);
        }
    }

    // Save other info
    outDeviceInfo.m_device = device;
    outDeviceInfo.m_dxgiFactory = dxgiFactory;
    outDeviceInfo.m_adapter = adapter;
    outDeviceInfo.m_isWarp = D3DUtil::isWarp(dxgiFactory, adapter);
    const UINT kMicrosoftVendorId = 5140;
    outDeviceInfo.m_isSoftware =
        outDeviceInfo.m_isWarp ||
        ((outDeviceInfo.m_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) ||
        outDeviceInfo.m_desc.VendorId == kMicrosoftVendorId;

    return SLANG_OK;
}

Result DeviceImpl::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    // Find extended desc.
    for (GfxIndex i = 0; i < desc.extendedDescCount; i++)
    {
        StructType stype;
        memcpy(&stype, desc.extendedDescs[i], sizeof(stype));
        if (stype == StructType::D3D12ExtendedDesc)
        {
            memcpy(&m_extendedDesc, desc.extendedDescs[i], sizeof(m_extendedDesc));
        }
    }

    // Initialize queue index allocator.
    // Support max 32 queues.
    m_queueIndexAllocator.initPool(32);

    // Initialize DeviceInfo
    {
        m_info.deviceType = DeviceType::DirectX12;
        m_info.bindingStyle = BindingStyle::DirectX;
        m_info.projectionStyle = ProjectionStyle::DirectX;
        m_info.apiName = "Direct3D 12";
        static const float kIdentity[] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

    // Get all the dll entry points
    m_D3D12SerializeRootSignature =
        (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)loadProc(d3dModule, "D3D12SerializeRootSignature");
    if (!m_D3D12SerializeRootSignature)
    {
        return SLANG_FAIL;
    }

    HMODULE pixModule = LoadLibraryW(L"WinPixEventRuntime.dll");
    if (pixModule)
    {
        m_BeginEventOnCommandList =
            (PFN_BeginEventOnCommandList)GetProcAddress(pixModule, "PIXBeginEventOnCommandList");
        m_EndEventOnCommandList =
            (PFN_EndEventOnCommandList)GetProcAddress(pixModule, "PIXEndEventOnCommandList");
    }

#if ENABLE_DEBUG_LAYER
    m_D3D12GetDebugInterface =
        (PFN_D3D12_GET_DEBUG_INTERFACE)loadProc(d3dModule, "D3D12GetDebugInterface");
    if (m_D3D12GetDebugInterface)
    {
        if (SLANG_SUCCEEDED(m_D3D12GetDebugInterface(IID_PPV_ARGS(m_dxDebug.writeRef()))))
        {
#    if 0
            // Can enable for extra validation. NOTE! That d3d12 warns if you do.... 
            // D3D12 MESSAGE : Device Debug Layer Startup Options : GPU - Based Validation is enabled(disabled by default).
            // This results in new validation not possible during API calls on the CPU, by creating patched shaders that have validation
            // added directly to the shader. However, it can slow things down a lot, especially for applications with numerous
            // PSOs.Time to see the first render frame may take several minutes.
            // [INITIALIZATION MESSAGE #1016: CREATEDEVICE_DEBUG_LAYER_STARTUP_OPTIONS]

            ComPtr<ID3D12Debug1> debug1;
            if (SLANG_SUCCEEDED(m_dxDebug->QueryInterface(debug1.writeRef())))
            {
                debug1->SetEnableGPUBasedValidation(true);
            }
#    endif
        }
    }
#endif

    m_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)loadProc(d3dModule, "D3D12CreateDevice");
    if (!m_D3D12CreateDevice)
    {
        return SLANG_FAIL;
    }

    if (desc.existingDeviceHandles.handles[0].handleValue == 0)
    {
        FlagCombiner combiner;
        // TODO: we should probably provide a command-line option
        // to override UseDebug of default rather than leave it
        // up to each back-end to specify.
#if ENABLE_DEBUG_LAYER
        combiner.add(
            DeviceCheckFlag::UseDebug, ChangeType::OnOff); ///< First try debug then non debug
#else
        combiner.add(DeviceCheckFlag::UseDebug, ChangeType::Off); ///< Don't bother with debug
#endif
        combiner.add(
            DeviceCheckFlag::UseHardwareDevice,
            ChangeType::OnOff); ///< First try hardware, then reference

        const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        const int numCombinations = combiner.getNumCombinations();
        for (int i = 0; i < numCombinations; ++i)
        {
            if (SLANG_SUCCEEDED(_createDevice(
                combiner.getCombination(i),
                UnownedStringSlice(desc.adapter),
                featureLevel,
                m_deviceInfo)))
            {
                break;
            }
        }

        if (!m_deviceInfo.m_adapter)
        {
            // Couldn't find an adapter
            return SLANG_FAIL;
        }
    }
    else
    {
        // Store the existing device handle in desc in m_deviceInfo
        m_deviceInfo.m_device = (ID3D12Device*)desc.existingDeviceHandles.handles[0].handleValue;
    }

    // Set the device
    m_device = m_deviceInfo.m_device;

    if (m_deviceInfo.m_isSoftware)
    {
        m_features.add("software-device");
    }
    else
    {
        m_features.add("hardware-device");
    }

    // NVAPI
    if (desc.nvapiExtnSlot >= 0)
    {
        if (SLANG_FAILED(NVAPIUtil::initialize()))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

#ifdef GFX_NVAPI
        // From DOCS: Applications are expected to bind null UAV to this slot.
        // NOTE! We don't currently do this, but doesn't seem to be a problem.

        const NvAPI_Status status =
            NvAPI_D3D12_SetNvShaderExtnSlotSpace(m_device, NvU32(desc.nvapiExtnSlot), NvU32(0));

        if (status != NVAPI_OK)
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        if (isSupportedNVAPIOp(m_device, NV_EXTN_OP_UINT64_ATOMIC))
        {
            m_features.add("atomic-int64");
        }
        if (isSupportedNVAPIOp(m_device, NV_EXTN_OP_FP32_ATOMIC))
        {
            m_features.add("atomic-float");
        }

        m_nvapi = true;
#endif
    }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModelData = {};
    shaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_6;

    // Find what features are supported
    {
        // Check this is how this is laid out...
        SLANG_COMPILE_TIME_ASSERT(D3D_SHADER_MODEL_6_0 == 0x60);

        {
            // TODO: Currently warp causes a crash when using half, so disable for now
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                D3D12_FEATURE_SHADER_MODEL, &shaderModelData, sizeof(shaderModelData))) &&
                m_deviceInfo.m_isWarp == false && shaderModelData.HighestShaderModel >= 0x62)
            {
                // With sm_6_2 we have half
                m_features.add("half");
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
            {
                // Check double precision support
                if (options.DoublePrecisionFloatShaderOps)
                    m_features.add("double");

                // Check conservative-rasterization support
                auto conservativeRasterTier = options.ConservativeRasterizationTier;
                if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_3)
                {
                    m_features.add("conservative-rasterization-3");
                    m_features.add("conservative-rasterization-2");
                    m_features.add("conservative-rasterization-1");
                }
                else if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_2)
                {
                    m_features.add("conservative-rasterization-2");
                    m_features.add("conservative-rasterization-1");
                }
                else if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_1)
                {
                    m_features.add("conservative-rasterization-1");
                }

                // Check rasterizer ordered views support
                if (options.ROVsSupported)
                {
                    m_features.add("rasterizer-ordered-views");
                }
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS2 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS2, &options, sizeof(options))))
            {
                // Check programmable sample positions support
                switch (options.ProgrammableSamplePositionsTier)
                {
                case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_2:
                    m_features.add("programmable-sample-positions-2");
                    m_features.add("programmable-sample-positions-1");
                    break;
                case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_1:
                    m_features.add("programmable-sample-positions-1");
                    break;
                default:
                    break;
                }
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS3 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS3, &options, sizeof(options))))
            {
                // Check barycentrics support
                if (options.BarycentricsSupported)
                {
                    m_features.add("barycentrics");
                }
            }
        }
        // Check ray tracing support
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options))))
            {
                if (options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
                {
                    m_features.add("ray-tracing");
                }
                if (options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1)
                {
                    m_features.add("ray-query");
                }
            }
        }
    }

    m_desc = desc;

    // Create a command queue for internal resource transfer operations.
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(m_resourceCommandQueue.writeRef()));
    // `CommandQueueImpl` holds a back reference to `D3D12Device`, make it a weak reference here
    // since this object is already owned by `D3D12Device`.
    m_resourceCommandQueue->breakStrongReferenceToDevice();
    // Retrieve timestamp frequency.
    m_resourceCommandQueue->m_d3dQueue->GetTimestampFrequency(&m_info.timestampFrequency);

    SLANG_RETURN_ON_FAIL(createTransientResourceHeapImpl(
        ITransientResourceHeap::Flags::AllowResizing,
        0,
        8,
        4,
        m_resourceCommandTransientHeap.writeRef()));
    // `TransientResourceHeap` holds a back reference to `D3D12Device`, make it a weak reference
    // here since this object is already owned by `D3D12Device`.
    m_resourceCommandTransientHeap->breakStrongReferenceToDevice();

    m_cpuViewHeap = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->init(
        m_device,
        1024 * 1024,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_cpuSamplerHeap = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap->init(
        m_device, 2048, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    m_rtvAllocator = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_rtvAllocator->init(
        m_device, 16 * 1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_dsvAllocator = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_dsvAllocator->init(
        m_device, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    ComPtr<IDXGIDevice> dxgiDevice;
    if (m_deviceInfo.m_adapter)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        m_deviceInfo.m_adapter->GetDesc(&adapterDesc);
        m_adapterName = String::fromWString(adapterDesc.Description);
        m_info.adapterName = m_adapterName.begin();
    }

    // Initialize DXR interface.
#if SLANG_GFX_HAS_DXR_SUPPORT
    m_device->QueryInterface<ID3D12Device5>(m_deviceInfo.m_device5.writeRef());
    m_device5 = m_deviceInfo.m_device5.get();
#endif
    // Check shader model version.
    SlangCompileTarget compileTarget = SLANG_DXBC;
    const char* profileName = "sm_5_1";
    switch (shaderModelData.HighestShaderModel)
    {
    case D3D_SHADER_MODEL_5_1:
        compileTarget = SLANG_DXBC;
        profileName = "sm_5_1";
        break;
    case D3D_SHADER_MODEL_6_0:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_0";
        break;
    case D3D_SHADER_MODEL_6_1:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_1";
        break;
    case D3D_SHADER_MODEL_6_2:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_2";
        break;
    case D3D_SHADER_MODEL_6_3:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_3";
        break;
    case D3D_SHADER_MODEL_6_4:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_4";
        break;
    case D3D_SHADER_MODEL_6_5:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_5";
        break;
    default:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_6";
        break;
    }
    m_features.add(profileName);
    // If user specified a higher shader model than what the system supports, return failure.
    int userSpecifiedShaderModel = D3DUtil::getShaderModelFromProfileName(desc.slang.targetProfile);
    if (userSpecifiedShaderModel > shaderModelData.HighestShaderModel)
    {
        getDebugCallback()->handleMessage(
            gfx::DebugMessageType::Error,
            gfx::DebugMessageSource::Layer,
            "The requested shader model is not supported by the system.");
        return SLANG_E_NOT_AVAILABLE;
    }
    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        compileTarget,
        profileName,
        makeArray(slang::PreprocessorMacroDesc{ "__D3D12__", "1" }).getView()));

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `DrawInstancedIndirect` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(drawIndirectCmdSignature.writeRef())));
    }

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `DrawIndexedInstancedIndirect` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(drawIndexedIndirectCmdSignature.writeRef())));
    }

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `Dispatch` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(dispatchIndirectCmdSignature.writeRef())));
    }
    m_isInitialized = true;
    return SLANG_OK;
}

Result DeviceImpl::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> heap;
    SLANG_RETURN_ON_FAIL(createTransientResourceHeapImpl(
        desc.flags,
        desc.constantBufferSize,
        getViewDescriptorCount(desc),
        Math::Max(1024, desc.samplerDescriptorCount),
        heap.writeRef()));
    returnComPtr(outHeap, heap);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    RefPtr<CommandQueueImpl> queue;
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(queue.writeRef()));
    returnComPtr(outQueue, queue);
    return SLANG_OK;
}

Result DeviceImpl::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    returnComPtr(outSwapchain, swapchain);
    return SLANG_OK;
}

SlangResult DeviceImpl::readTextureResource(
    ITextureResource* resource,
    ResourceState state,
    ISlangBlob** outBlob,
    Size* outRowPitch,
    Size* outPixelSize)
{
    return captureTextureToSurface(
        static_cast<TextureResourceImpl*>(resource), state, outBlob, outRowPitch, outPixelSize);
}

Result DeviceImpl::getTextureAllocationInfo(
    const ITextureResource::Desc& desc, Size* outSize, Size* outAlignment)
{
    TextureResource::Desc srcDesc = fixupTextureDesc(desc);
    D3D12_RESOURCE_DESC resourceDesc = {};
    initTextureResourceDesc(resourceDesc, srcDesc);
    auto allocInfo = m_device->GetResourceAllocationInfo(0, 1, &resourceDesc);
    *outSize = (Size)allocInfo.SizeInBytes;
    *outAlignment = (Size)allocInfo.Alignment;
    return SLANG_OK;
}

Result DeviceImpl::getTextureRowAlignment(Size* outAlignment)
{
    *outAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    return SLANG_OK;
}

Result DeviceImpl::createTextureResource(
    const ITextureResource::Desc& descIn,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
{
    // Description of uploading on Dx12
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215%28v=vs.85%29.aspx

    TextureResource::Desc srcDesc = fixupTextureDesc(descIn);

    D3D12_RESOURCE_DESC resourceDesc = {};
    initTextureResourceDesc(resourceDesc, srcDesc);
    const int arraySize = calcEffectiveArraySize(srcDesc);
    const int numMipMaps = srcDesc.numMipLevels;

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));

    // Create the target resource
    {
        D3D12_HEAP_PROPERTIES heapProps;

        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
        if (descIn.isShared)
            flags |= D3D12_HEAP_FLAG_SHARED;

        D3D12_CLEAR_VALUE clearValue;
        D3D12_CLEAR_VALUE* clearValuePtr = &clearValue;
        if ((resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
        {
            clearValuePtr = nullptr;
        }
        if (isTypelessDepthFormat(resourceDesc.Format))
        {
            clearValuePtr = nullptr;
        }
        clearValue.Format = resourceDesc.Format;
        memcpy(clearValue.Color, &descIn.optimalClearValue.color, sizeof(clearValue.Color));
        clearValue.DepthStencil.Depth = descIn.optimalClearValue.depthStencil.depth;
        clearValue.DepthStencil.Stencil = descIn.optimalClearValue.depthStencil.stencil;
        SLANG_RETURN_ON_FAIL(texture->m_resource.initCommitted(
            m_device,
            heapProps,
            flags,
            resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            clearValuePtr));

        texture->m_resource.setDebugName(L"Texture");
    }

    // Calculate the layout
    List<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    layouts.setCount(numMipMaps);
    List<UInt64> mipRowSizeInBytes;
    mipRowSizeInBytes.setCount(srcDesc.numMipLevels);
    List<UInt32> mipNumRows;
    mipNumRows.setCount(numMipMaps);

    // NOTE! This is just the size for one array upload -> not for the whole texture
    UInt64 requiredSize = 0;
    m_device->GetCopyableFootprints(
        &resourceDesc,
        0,
        srcDesc.numMipLevels,
        0,
        layouts.begin(),
        mipNumRows.begin(),
        mipRowSizeInBytes.begin(),
        &requiredSize);

    // Sub resource indexing
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx#subresource_indexing
    if (initData)
    {
        // Create the upload texture
        D3D12Resource uploadTexture;

        {
            D3D12_HEAP_PROPERTIES heapProps;

            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC uploadResourceDesc;

            uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            uploadResourceDesc.Width = requiredSize;
            uploadResourceDesc.Height = 1;
            uploadResourceDesc.DepthOrArraySize = 1;
            uploadResourceDesc.MipLevels = 1;
            uploadResourceDesc.SampleDesc.Count = 1;
            uploadResourceDesc.SampleDesc.Quality = 0;
            uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            uploadResourceDesc.Alignment = 0;

            SLANG_RETURN_ON_FAIL(uploadTexture.initCommitted(
                m_device,
                heapProps,
                D3D12_HEAP_FLAG_NONE,
                uploadResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr));

            uploadTexture.setDebugName(L"TextureUpload");
        }
        // Get the pointer to the upload resource
        ID3D12Resource* uploadResource = uploadTexture;

        int subResourceIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
        {
            uint8_t* p;
            uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&p));

            for (int j = 0; j < numMipMaps; ++j)
            {
                auto srcSubresource = initData[subResourceIndex + j];

                const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[j];
                const D3D12_SUBRESOURCE_FOOTPRINT& footprint = layout.Footprint;

                TextureResource::Extents mipSize = calcMipSize(srcDesc.size, j);
                if (gfxIsCompressedFormat(descIn.format))
                {
                    mipSize.width = int(D3DUtil::calcAligned(mipSize.width, 4));
                    mipSize.height = int(D3DUtil::calcAligned(mipSize.height, 4));
                }

                assert(
                    footprint.Width == mipSize.width && footprint.Height == mipSize.height &&
                    footprint.Depth == mipSize.depth);

                auto mipRowSize = mipRowSizeInBytes[j];

                const ptrdiff_t dstMipRowPitch = ptrdiff_t(footprint.RowPitch);
                const ptrdiff_t srcMipRowPitch = ptrdiff_t(srcSubresource.strideY);

                const ptrdiff_t dstMipLayerPitch = ptrdiff_t(footprint.RowPitch * footprint.Height);
                const ptrdiff_t srcMipLayerPitch = ptrdiff_t(srcSubresource.strideZ);

                // Our outer loop will copy the depth layers one at a time.
                //
                const uint8_t* srcLayer = (const uint8_t*)srcSubresource.data;
                uint8_t* dstLayer = p + layouts[j].Offset;
                for (int l = 0; l < mipSize.depth; l++)
                {
                    // Our inner loop will copy the rows one at a time.
                    //
                    const uint8_t* srcRow = srcLayer;
                    uint8_t* dstRow = dstLayer;
                    int j = gfxIsCompressedFormat(descIn.format)
                        ? 4
                        : 1; // BC compressed formats are organized into 4x4 blocks
                    for (int k = 0; k < mipSize.height; k += j)
                    {
                        ::memcpy(dstRow, srcRow, (Size)mipRowSize);

                        srcRow += srcMipRowPitch;
                        dstRow += dstMipRowPitch;
                    }

                    srcLayer += srcMipLayerPitch;
                    dstLayer += dstMipLayerPitch;
                }

                // assert(srcRow == (const uint8_t*)(srcMip.getBuffer() + srcMip.getCount()));
            }
            uploadResource->Unmap(0, nullptr);

            auto encodeInfo = encodeResourceCommands();
            for (int mipIndex = 0; mipIndex < numMipMaps; ++mipIndex)
            {
                // https://msdn.microsoft.com/en-us/library/windows/desktop/dn903862(v=vs.85).aspx

                D3D12_TEXTURE_COPY_LOCATION src;
                src.pResource = uploadTexture;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = layouts[mipIndex];

                D3D12_TEXTURE_COPY_LOCATION dst;
                dst.pResource = texture->m_resource;
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dst.SubresourceIndex = subResourceIndex;
                encodeInfo.d3dCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

                subResourceIndex++;
            }

            // Block - waiting for copy to complete (so can drop upload texture)
            submitResourceCommandsAndWait(encodeInfo);
        }
    }
    {
        auto encodeInfo = encodeResourceCommands();
        {
            D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
            texture->m_resource.transition(
                D3D12_RESOURCE_STATE_COPY_DEST, texture->m_defaultState, submitter);
        }
        submitResourceCommandsAndWait(encodeInfo);
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result DeviceImpl::createTextureFromNativeHandle(
    InteropHandle handle, const ITextureResource::Desc& srcDesc, ITextureResource** outResource)
{
    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));

    if (handle.api == InteropHandleAPI::D3D12)
    {
        texture->m_resource.setResource((ID3D12Resource*)handle.handleValue);
    }
    else
    {
        return SLANG_FAIL;
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result DeviceImpl::createBufferResource(
    const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc srcDesc = fixupBufferDesc(descIn);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    D3D12_RESOURCE_DESC bufferDesc;
    initBufferResourceDesc(descIn.sizeInBytes, bufferDesc);

    bufferDesc.Flags |= calcResourceFlags(srcDesc.allowedStates);

    const D3D12_RESOURCE_STATES initialState = buffer->m_defaultState;
    SLANG_RETURN_ON_FAIL(createBuffer(
        bufferDesc,
        initData,
        srcDesc.sizeInBytes,
        initialState,
        buffer->m_resource,
        descIn.isShared,
        descIn.memoryType));

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createBufferFromNativeHandle(
    InteropHandle handle, const IBufferResource::Desc& srcDesc, IBufferResource** outResource)
{
    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    if (handle.api == InteropHandleAPI::D3D12)
    {
        buffer->m_resource.setResource((ID3D12Resource*)handle.handleValue);
    }
    else
    {
        return SLANG_FAIL;
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    D3D12_FILTER_REDUCTION_TYPE dxReduction = translateFilterReduction(desc.reductionOp);
    D3D12_FILTER dxFilter;
    if (desc.maxAnisotropy > 1)
    {
        dxFilter = D3D12_ENCODE_ANISOTROPIC_FILTER(dxReduction);
    }
    else
    {
        D3D12_FILTER_TYPE dxMin = translateFilterMode(desc.minFilter);
        D3D12_FILTER_TYPE dxMag = translateFilterMode(desc.magFilter);
        D3D12_FILTER_TYPE dxMip = translateFilterMode(desc.mipFilter);

        dxFilter = D3D12_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, dxReduction);
    }

    D3D12_SAMPLER_DESC dxDesc = {};
    dxDesc.Filter = dxFilter;
    dxDesc.AddressU = translateAddressingMode(desc.addressU);
    dxDesc.AddressV = translateAddressingMode(desc.addressV);
    dxDesc.AddressW = translateAddressingMode(desc.addressW);
    dxDesc.MipLODBias = desc.mipLODBias;
    dxDesc.MaxAnisotropy = desc.maxAnisotropy;
    dxDesc.ComparisonFunc = translateComparisonFunc(desc.comparisonFunc);
    for (int ii = 0; ii < 4; ++ii)
        dxDesc.BorderColor[ii] = desc.borderColor[ii];
    dxDesc.MinLOD = desc.minLOD;
    dxDesc.MaxLOD = desc.maxLOD;

    auto& samplerHeap = m_cpuSamplerHeap;

    D3D12Descriptor cpuDescriptor;
    samplerHeap->allocate(&cpuDescriptor);
    m_device->CreateSampler(&dxDesc, cpuDescriptor.cpuHandle);

    // TODO: We really ought to have a free-list of sampler-heap
    // entries that we check before we go to the heap, and then
    // when we are done with a sampler we simply add it to the free list.
    //
    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_allocator = samplerHeap;
    samplerImpl->m_descriptor = cpuDescriptor;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result DeviceImpl::createTextureView(
    ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*)texture;

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;
    bool isArray = resourceImpl ? resourceImpl->getDesc()->arraySize != 0 : false;
    bool isMultiSample = resourceImpl ? resourceImpl->getDesc()->sampleDesc.numSamples > 1 : false;
    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::RenderTarget:
    {
        SLANG_RETURN_ON_FAIL(m_rtvAllocator->allocate(&viewImpl->m_descriptor));
        viewImpl->m_allocator = m_rtvAllocator;
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = D3DUtil::getMapFormat(desc.format);
        isArray = desc.subresourceRange.layerCount > 1;
        switch (desc.renderTarget.shape)
        {
        case IResource::Type::Texture1D:
            rtvDesc.ViewDimension =
                isArray ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
            break;
        case IResource::Type::Texture2D:
            if (isMultiSample)
            {
                rtvDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY
                    : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                rtvDesc.Texture2DMSArray.ArraySize = desc.subresourceRange.layerCount;
                rtvDesc.Texture2DMSArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
            }
            else
            {
                rtvDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY
                    : D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2DArray.MipSlice = desc.subresourceRange.mipLevel;
                rtvDesc.Texture2DArray.PlaneSlice =
                    resourceImpl ? D3DUtil::getPlaneSlice(
                        D3DUtil::getMapFormat(resourceImpl->getDesc()->format),
                        desc.subresourceRange.aspectMask)
                    : 0;
                rtvDesc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount;
                rtvDesc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
            }
            break;
        case IResource::Type::Texture3D:
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = desc.subresourceRange.mipLevel;
            rtvDesc.Texture3D.FirstWSlice = desc.subresourceRange.baseArrayLayer;
            rtvDesc.Texture3D.WSize = (desc.subresourceRange.layerCount == 0) ? -1 : desc.subresourceRange.layerCount;
            break;
        case IResource::Type::Buffer:
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
            break;
        default:
            return SLANG_FAIL;
        }
        m_device->CreateRenderTargetView(
            resourceImpl ? resourceImpl->m_resource.getResource() : nullptr,
            &rtvDesc,
            viewImpl->m_descriptor.cpuHandle);
    }
    break;

    case IResourceView::Type::DepthStencil:
    {
        SLANG_RETURN_ON_FAIL(m_dsvAllocator->allocate(&viewImpl->m_descriptor));
        viewImpl->m_allocator = m_dsvAllocator;
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = D3DUtil::getMapFormat(desc.format);
        isArray = desc.subresourceRange.layerCount > 1;
        switch (desc.renderTarget.shape)
        {
        case IResource::Type::Texture1D:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
            break;
        case IResource::Type::Texture2D:
            if (isMultiSample)
            {
                dsvDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY
                    : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                dsvDesc.Texture2DMSArray.ArraySize = desc.subresourceRange.layerCount;
                dsvDesc.Texture2DMSArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
            }
            else
            {
                dsvDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY
                    : D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2DArray.MipSlice = desc.subresourceRange.mipLevel;
                dsvDesc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount;
                dsvDesc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
            }
            break;
        default:
            return SLANG_FAIL;
        }
        m_device->CreateDepthStencilView(
            resourceImpl ? resourceImpl->m_resource.getResource() : nullptr,
            &dsvDesc,
            viewImpl->m_descriptor.cpuHandle);
    }
    break;

    case IResourceView::Type::UnorderedAccess:
    {
        // TODO: need to support the separate "counter resource" for the case
        // of append/consume buffers with attached counters.

        SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
        viewImpl->m_allocator = m_cpuViewHeap;
        D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12desc = {};
        auto& resourceDesc = *resourceImpl->getDesc();
        d3d12desc.Format = gfxIsTypelessFormat(texture->getDesc()->format)
            ? D3DUtil::getMapFormat(desc.format)
            : D3DUtil::getMapFormat(texture->getDesc()->format);
        switch (resourceImpl->getDesc()->type)
        {
        case IResource::Type::Texture1D:
            d3d12desc.ViewDimension = resourceDesc.arraySize == 0
                ? D3D12_UAV_DIMENSION_TEXTURE1D
                : D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            d3d12desc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
            d3d12desc.Texture1DArray.ArraySize = desc.subresourceRange.layerCount == 0
                ? resourceDesc.arraySize
                : desc.subresourceRange.layerCount;
            d3d12desc.Texture1DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;

            break;
        case IResource::Type::Texture2D:
            d3d12desc.ViewDimension = resourceDesc.arraySize == 0
                ? D3D12_UAV_DIMENSION_TEXTURE2D
                : D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            d3d12desc.Texture2D.MipSlice = desc.subresourceRange.mipLevel;
            d3d12desc.Texture2D.PlaneSlice =
                D3DUtil::getPlaneSlice(d3d12desc.Format, desc.subresourceRange.aspectMask);
            d3d12desc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount == 0
                ? resourceDesc.arraySize
                : desc.subresourceRange.layerCount;
            d3d12desc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
            break;
        case IResource::Type::Texture3D:
            d3d12desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            d3d12desc.Texture3D.MipSlice = desc.subresourceRange.mipLevel;
            d3d12desc.Texture3D.FirstWSlice = desc.subresourceRange.baseArrayLayer;
            d3d12desc.Texture3D.WSize = resourceDesc.size.depth;
            break;
        default:
            return SLANG_FAIL;
        }
        m_device->CreateUnorderedAccessView(
            resourceImpl->m_resource, nullptr, &d3d12desc, viewImpl->m_descriptor.cpuHandle);
    }
    break;

    case IResourceView::Type::ShaderResource:
    {
        SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
        viewImpl->m_allocator = m_cpuViewHeap;

        // Need to construct the D3D12_SHADER_RESOURCE_VIEW_DESC because otherwise TextureCube
        // is not accessed appropriately (rather than just passing nullptr to
        // CreateShaderResourceView)
        const D3D12_RESOURCE_DESC resourceDesc =
            resourceImpl->m_resource.getResource()->GetDesc();
        const DXGI_FORMAT pixelFormat = desc.format == Format::Unknown
            ? resourceDesc.Format
            : D3DUtil::getMapFormat(desc.format);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        initSrvDesc(
            resourceImpl->getType(),
            *resourceImpl->getDesc(),
            resourceDesc,
            pixelFormat,
            desc.subresourceRange,
            srvDesc);

        m_device->CreateShaderResourceView(
            resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
    }
    break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result DeviceImpl::getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT support;
    support.Format = D3DUtil::getMapFormat(format);
    SLANG_RETURN_ON_FAIL(
        m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support)));

    ResourceStateSet allowedStates;

    auto dxgi1 = support.Support1;
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_BUFFER)
        allowedStates.add(ResourceState::ConstantBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
        allowedStates.add(ResourceState::VertexBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_IA_INDEX_BUFFER)
        allowedStates.add(ResourceState::IndexBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SO_BUFFER)
        allowedStates.add(ResourceState::StreamOutput);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_GATHER)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
        allowedStates.add(ResourceState::RenderTarget);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        allowedStates.add(ResourceState::DepthWrite);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)
        allowedStates.add(ResourceState::UnorderedAccess);

    *outStates = allowedStates;
    return SLANG_OK;
}

Result DeviceImpl::createBufferView(
    IBufferResource* buffer,
    IBufferResource* counterBuffer,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*)buffer;
    auto resourceDesc = *resourceImpl->getDesc();

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Format = D3DUtil::getMapFormat(desc.format);
        uavDesc.Buffer.FirstElement = desc.bufferRange.firstElement;
        uint64_t viewSize = 0;
        if (desc.bufferElementSize)
        {
            uavDesc.Buffer.StructureByteStride = (UINT)desc.bufferElementSize;
            uavDesc.Buffer.NumElements =
                desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / desc.bufferElementSize)
                : (UINT)desc.bufferRange.elementCount;
            viewSize = (uint64_t)desc.bufferElementSize * uavDesc.Buffer.NumElements;
        }
        else if (desc.format == Format::Unknown)
        {
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.Buffer.NumElements = desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / 4)
                : UINT(desc.bufferRange.elementCount / 4);
            uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
            viewSize = 4ull * uavDesc.Buffer.NumElements;
        }
        else
        {
            FormatInfo sizeInfo;
            gfxGetFormatInfo(desc.format, &sizeInfo);
            assert(sizeInfo.pixelsPerBlock == 1);
            uavDesc.Buffer.NumElements =
                desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / sizeInfo.blockSizeInBytes)
                : (UINT)desc.bufferRange.elementCount;
            viewSize = (uint64_t)uavDesc.Buffer.NumElements * sizeInfo.blockSizeInBytes;
        }

        if (viewSize >= (1ull << 32) - 8)
        {
            // D3D12 does not support view descriptors that has size near 4GB.
            // We will not create actual SRV/UAVs for such large buffers.
            // However, a buffer this large can still be bound as root parameter.
            // So instead of failing, we quietly ignore descriptor creation.
            viewImpl->m_descriptor.cpuHandle.ptr = 0;
        }
        else
        {
            auto counterResourceImpl = static_cast<BufferResourceImpl*>(counterBuffer);
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            m_device->CreateUnorderedAccessView(
                resourceImpl->m_resource,
                counterResourceImpl ? counterResourceImpl->m_resource.getResource() : nullptr,
                &uavDesc,
                viewImpl->m_descriptor.cpuHandle);
        }
    }
    break;

    case IResourceView::Type::ShaderResource:
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = D3DUtil::getMapFormat(desc.format);
        srvDesc.Buffer.StructureByteStride = 0;
        srvDesc.Buffer.FirstElement = desc.bufferRange.firstElement;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        uint64_t viewSize = 0;
        if (desc.bufferElementSize)
        {
            srvDesc.Buffer.StructureByteStride = (UINT)desc.bufferElementSize;
            srvDesc.Buffer.NumElements =
                desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / desc.bufferElementSize)
                : (UINT)desc.bufferRange.elementCount;
            viewSize = (uint64_t)desc.bufferElementSize * srvDesc.Buffer.NumElements;
        }
        else if (desc.format == Format::Unknown)
        {
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Buffer.NumElements = desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / 4)
                : UINT(desc.bufferRange.elementCount / 4);
            srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
            viewSize = 4ull * srvDesc.Buffer.NumElements;
        }
        else
        {
            FormatInfo sizeInfo;
            gfxGetFormatInfo(desc.format, &sizeInfo);
            assert(sizeInfo.pixelsPerBlock == 1);
            srvDesc.Buffer.NumElements =
                desc.bufferRange.elementCount == 0
                ? UINT(resourceDesc.sizeInBytes / sizeInfo.blockSizeInBytes)
                : (UINT)desc.bufferRange.elementCount;
            viewSize = (uint64_t)srvDesc.Buffer.NumElements * sizeInfo.blockSizeInBytes;
        }
        if (viewSize >= (1ull << 32) - 8)
        {
            // D3D12 does not support view descriptors that has size near 4GB.
            // We will not create actual SRV/UAVs for such large buffers.
            // However, a buffer this large can still be bound as root parameter.
            // So instead of failing, we quietly ignore descriptor creation.
            viewImpl->m_descriptor.cpuHandle.ptr = 0;
        }
        else
        {
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            m_device->CreateShaderResourceView(
                resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
    }
    break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result DeviceImpl::createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFb)
{
    RefPtr<FramebufferImpl> framebuffer = new FramebufferImpl();
    framebuffer->renderTargetViews.setCount(desc.renderTargetCount);
    framebuffer->renderTargetDescriptors.setCount(desc.renderTargetCount);
    framebuffer->renderTargetClearValues.setCount(desc.renderTargetCount);
    for (GfxIndex i = 0; i < desc.renderTargetCount; i++)
    {
        framebuffer->renderTargetViews[i] =
            static_cast<ResourceViewImpl*>(desc.renderTargetViews[i]);
        framebuffer->renderTargetDescriptors[i] =
            framebuffer->renderTargetViews[i]->m_descriptor.cpuHandle;
        if (static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
        {
            auto clearValue =
                static_cast<TextureResourceImpl*>(
                    static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
                ->getDesc()
                ->optimalClearValue.color;
            memcpy(&framebuffer->renderTargetClearValues[i], &clearValue, sizeof(ColorClearValue));
        }
        else
        {
            memset(&framebuffer->renderTargetClearValues[i], 0, sizeof(ColorClearValue));
        }
    }
    framebuffer->depthStencilView = static_cast<ResourceViewImpl*>(desc.depthStencilView);
    if (desc.depthStencilView)
    {
        framebuffer->depthStencilClearValue =
            static_cast<TextureResourceImpl*>(
                static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_resource.Ptr())
            ->getDesc()
            ->optimalClearValue.depthStencil;
        framebuffer->depthStencilDescriptor =
            static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_descriptor.cpuHandle;
    }
    else
    {
        framebuffer->depthStencilDescriptor.ptr = 0;
    }
    returnComPtr(outFb, framebuffer);
    return SLANG_OK;
}

Result DeviceImpl::createFramebufferLayout(
    IFramebufferLayout::Desc const& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    layout->m_renderTargets.setCount(desc.renderTargetCount);
    for (GfxIndex i = 0; i < desc.renderTargetCount; i++)
    {
        layout->m_renderTargets[i] = desc.renderTargets[i];
    }

    if (desc.depthStencil)
    {
        layout->m_hasDepthStencil = true;
        layout->m_depthStencil = *desc.depthStencil;
    }
    else
    {
        layout->m_hasDepthStencil = false;
    }
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    result->init(desc);
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result DeviceImpl::createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    // Work out a buffer size to hold all text
    Size textSize = 0;
    auto inputElementCount = desc.inputElementCount;
    auto inputElements = desc.inputElements;
    auto vertexStreamCount = desc.vertexStreamCount;
    auto vertexStreams = desc.vertexStreams;
    for (int i = 0; i < Int(inputElementCount); ++i)
    {
        const char* text = inputElements[i].semanticName;
        textSize += text ? (::strlen(text) + 1) : 0;
    }
    layout->m_text.setCount(textSize);
    char* textPos = layout->m_text.getBuffer();

    List<D3D12_INPUT_ELEMENT_DESC>& elements = layout->m_elements;
    elements.setCount(inputElementCount);

    for (Int i = 0; i < inputElementCount; ++i)
    {
        const InputElementDesc& srcEle = inputElements[i];
        const auto& srcStream = vertexStreams[srcEle.bufferSlotIndex];
        D3D12_INPUT_ELEMENT_DESC& dstEle = elements[i];

        // Add text to the buffer
        const char* semanticName = srcEle.semanticName;
        if (semanticName)
        {
            const int len = int(::strlen(semanticName));
            ::memcpy(textPos, semanticName, len + 1);
            semanticName = textPos;
            textPos += len + 1;
        }

        dstEle.SemanticName = semanticName;
        dstEle.SemanticIndex = (UINT)srcEle.semanticIndex;
        dstEle.Format = D3DUtil::getMapFormat(srcEle.format);
        dstEle.InputSlot = (UINT)srcEle.bufferSlotIndex;
        dstEle.AlignedByteOffset = (UINT)srcEle.offset;
        dstEle.InputSlotClass = D3DUtil::getInputSlotClass(srcStream.slotClass);
        dstEle.InstanceDataStepRate = (UINT)srcStream.instanceDataStepRate;
    }

    auto& vertexStreamStrides = layout->m_vertexStreamStrides;
    vertexStreamStrides.setCount(vertexStreamCount);
    for (GfxIndex i = 0; i < vertexStreamCount; ++i)
    {
        vertexStreamStrides[i] = (UINT)vertexStreams[i].stride;
    }

    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

const gfx::DeviceInfo& DeviceImpl::getDeviceInfo() const { return m_info; }

Result DeviceImpl::readBufferResource(
    IBufferResource* bufferIn, Offset offset, Size size, ISlangBlob** outBlob)
{

    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);

    const Size bufferSize = buffer->getDesc()->sizeInBytes;

    // This will be slow!!! - it blocks CPU on GPU completion
    D3D12Resource& resource = buffer->m_resource;

    D3D12Resource stageBuf;
    if (buffer->getDesc()->memoryType != MemoryType::ReadBack)
    {
        auto encodeInfo = encodeResourceCommands();

        // Readback heap
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        // Resource to readback to
        D3D12_RESOURCE_DESC stagingDesc;
        initBufferResourceDesc(size, stagingDesc);

        SLANG_RETURN_ON_FAIL(stageBuf.initCommitted(
            m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));

        // Do the copy
        encodeInfo.d3dCommandList->CopyBufferRegion(stageBuf, 0, resource, offset, size);

        // Wait until complete
        submitResourceCommandsAndWait(encodeInfo);
    }

    D3D12Resource& stageBufRef =
        buffer->getDesc()->memoryType != MemoryType::ReadBack ? stageBuf : resource;

    // Map and copy
    RefPtr<ListBlob> blob = new ListBlob();
    {
        UINT8* data;
        D3D12_RANGE readRange = { 0, size };

        SLANG_RETURN_ON_FAIL(
            stageBufRef.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        // Copy to memory buffer
        blob->m_data.setCount(size);
        ::memcpy(blob->m_data.getBuffer(), data, size);

        stageBufRef.getResource()->Unmap(0, nullptr);
    }
    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

Result DeviceImpl::createProgram(
    const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnosticBlob)
{
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
    shaderProgram->init(desc);
    ComPtr<ID3DBlob> d3dDiagnosticBlob;
    auto rootShaderLayoutResult = RootShaderObjectLayoutImpl::create(
        this,
        shaderProgram->linkedProgram,
        shaderProgram->linkedProgram->getLayout(),
        shaderProgram->m_rootObjectLayout.writeRef(),
        d3dDiagnosticBlob.writeRef());
    if (!SLANG_SUCCEEDED(rootShaderLayoutResult))
    {
        if (outDiagnosticBlob && d3dDiagnosticBlob)
        {
            RefPtr<StringBlob> diagnosticBlob =
                new StringBlob(String((const char*)d3dDiagnosticBlob->GetBufferPointer()));
            returnComPtr(outDiagnosticBlob, diagnosticBlob);
        }
        return rootShaderLayoutResult;
    }
    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout, ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, reinterpret_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result DeviceImpl::createMutableShaderObject(
    ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    auto result = createShaderObject(layout, outObject);
    SLANG_RETURN_ON_FAIL(result);
    static_cast<ShaderObjectImpl*>(*outObject)->m_isMutable = true;
    return result;
}

Result DeviceImpl::createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    RefPtr<MutableRootShaderObjectImpl> result = new MutableRootShaderObjectImpl();
    result->init(this);
    auto programImpl = static_cast<ShaderProgramImpl*>(program);
    result->resetImpl(
        this, programImpl->m_rootObjectLayout, m_cpuViewHeap.Ptr(), m_cpuSamplerHeap.Ptr(), true);
    returnComPtr(outObject, result);
    return SLANG_OK;
}

Result DeviceImpl::createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable)
{
    RefPtr<ShaderTableImpl> result = new ShaderTableImpl();
    result->m_device = this;
    result->init(desc);
    returnComPtr(outShaderTable, result);
    return SLANG_OK;
}

Result DeviceImpl::createGraphicsPipelineState(
    const GraphicsPipelineStateDesc& desc, IPipelineState** outState)
{
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createComputePipelineState(
    const ComputePipelineStateDesc& desc, IPipelineState** outState)
{
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

DeviceImpl::ResourceCommandRecordInfo DeviceImpl::encodeResourceCommands()
{
    ResourceCommandRecordInfo info;
    m_resourceCommandTransientHeap->createCommandBuffer(info.commandBuffer.writeRef());
    info.d3dCommandList = static_cast<CommandBufferImpl*>(info.commandBuffer.get())->m_cmdList;
    return info;
}

void DeviceImpl::submitResourceCommandsAndWait(const DeviceImpl::ResourceCommandRecordInfo& info)
{
    info.commandBuffer->close();
    m_resourceCommandQueue->executeCommandBuffer(info.commandBuffer);
    m_resourceCommandTransientHeap->finish();
    m_resourceCommandTransientHeap->synchronizeAndReset();
}

Result DeviceImpl::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outState)
{
    switch (desc.type)
    {
    case QueryType::AccelerationStructureCompactedSize:
    case QueryType::AccelerationStructureSerializedSize:
    case QueryType::AccelerationStructureCurrentSize:
    {
        RefPtr<PlainBufferProxyQueryPoolImpl> queryPoolImpl =
            new PlainBufferProxyQueryPoolImpl();
        uint32_t stride = 8;
        if (desc.type == QueryType::AccelerationStructureSerializedSize)
            stride = 16;
        SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this, stride));
        returnComPtr(outState, queryPoolImpl);
        return SLANG_OK;
    }
    default:
    {
        RefPtr<QueryPoolImpl> queryPoolImpl = new QueryPoolImpl();
        SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this));
        returnComPtr(outState, queryPoolImpl);
        return SLANG_OK;
    }
    }
}

Result DeviceImpl::createFence(const IFence::Desc& desc, IFence** outFence)
{
    RefPtr<FenceImpl> fence = new FenceImpl();
    SLANG_RETURN_ON_FAIL(fence->init(this, desc));
    returnComPtr(outFence, fence);
    return SLANG_OK;
}

Result DeviceImpl::waitForFences(
    GfxCount fenceCount, IFence** fences, uint64_t* fenceValues, bool waitForAll, uint64_t timeout)
{
    ShortList<HANDLE> waitHandles;
    for (GfxCount i = 0; i < fenceCount; ++i)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fences[i]);
        waitHandles.add(fenceImpl->getWaitEvent());
        SLANG_RETURN_ON_FAIL(
            fenceImpl->m_fence->SetEventOnCompletion(fenceValues[i], fenceImpl->getWaitEvent()));
    }
    auto result = WaitForMultipleObjects(
        fenceCount,
        waitHandles.getArrayView().getBuffer(),
        waitForAll ? TRUE : FALSE,
        timeout == kTimeoutInfinite ? INFINITE : (DWORD)(timeout / 1000000));
    if (result == WAIT_TIMEOUT)
        return SLANG_E_TIME_OUT;
    return result == WAIT_FAILED ? SLANG_FAIL : SLANG_OK;
}

Result DeviceImpl::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    if (!m_device5)
        return SLANG_E_NOT_AVAILABLE;

    D3DAccelerationStructureInputsBuilder inputsBuilder;
    SLANG_RETURN_ON_FAIL(inputsBuilder.build(buildInputs, getDebugCallback()));

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    m_device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputsBuilder.desc, &prebuildInfo);

    outPrebuildInfo->resultDataMaxSize = (Size)prebuildInfo.ResultDataMaxSizeInBytes;
    outPrebuildInfo->scratchDataSize = (Size)prebuildInfo.ScratchDataSizeInBytes;
    outPrebuildInfo->updateScratchDataSize = (Size)prebuildInfo.UpdateScratchDataSizeInBytes;
    return SLANG_OK;
}

Result DeviceImpl::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outAS)
{
#if SLANG_GFX_HAS_DXR_SUPPORT
    RefPtr<AccelerationStructureImpl> result = new AccelerationStructureImpl();
    result->m_device5 = m_device5;
    result->m_buffer = static_cast<BufferResourceImpl*>(desc.buffer);
    result->m_size = desc.size;
    result->m_offset = desc.offset;
    result->m_allocator = m_cpuViewHeap;
    result->m_desc.type = IResourceView::Type::AccelerationStructure;
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&result->m_descriptor));
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location =
        result->m_buffer->getDeviceAddress() + desc.offset;
    m_device->CreateShaderResourceView(nullptr, &srvDesc, result->m_descriptor.cpuHandle);
    returnComPtr(outAS, result);
    return SLANG_OK;
#else
    * outAS = nullptr;
    return SLANG_FAIL;
#endif
}

Result DeviceImpl::createRayTracingPipelineState(
    const RayTracingPipelineStateDesc& inDesc, IPipelineState** outState)
{
    if (!m_device5)
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    RefPtr<RayTracingPipelineStateImpl> pipelineStateImpl = new RayTracingPipelineStateImpl(this);
    pipelineStateImpl->init(inDesc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createTransientResourceHeapImpl(
    ITransientResourceHeap::Flags::Enum flags,
    Size constantBufferSize,
    uint32_t viewDescriptors,
    uint32_t samplerDescriptors,
    TransientResourceHeapImpl** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    ITransientResourceHeap::Desc desc = {};
    desc.flags = flags;
    desc.samplerDescriptorCount = samplerDescriptors;
    desc.constantBufferSize = constantBufferSize;
    desc.constantBufferDescriptorCount = viewDescriptors;
    desc.accelerationStructureDescriptorCount = viewDescriptors;
    desc.srvDescriptorCount = viewDescriptors;
    desc.uavDescriptorCount = viewDescriptors;
    SLANG_RETURN_ON_FAIL(result->init(desc, this, viewDescriptors, samplerDescriptors));
    returnRefPtrMove(outHeap, result);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueueImpl(CommandQueueImpl** outQueue)
{
    int queueIndex = m_queueIndexAllocator.alloc(1);
    // If we run out of queue index space, then the user is requesting too many queues.
    if (queueIndex == -1)
        return SLANG_FAIL;

    RefPtr<CommandQueueImpl> queue = new CommandQueueImpl();
    SLANG_RETURN_ON_FAIL(queue->init(this, (uint32_t)queueIndex));
    returnRefPtrMove(outQueue, queue);
    return SLANG_OK;
}

PROC DeviceImpl::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

DeviceImpl::~DeviceImpl() { m_shaderObjectLayoutCache = decltype(m_shaderObjectLayoutCache)(); }


} // namespace d3d12
} // namespace gfx
