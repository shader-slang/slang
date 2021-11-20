#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void sharedTextureTestImpl(IDevice* srcDevice, IDevice* dstDevice, UnitTestContext* context)
    {
        // Create a shareable texture using srcDevice, get its handle, then create a texture using the handle using
        // dstDevice. Read back the texture and check that its contents are correct.
        uint8_t texData[] = { 0u, 0u, 0u, 255u, 127u, 127u, 127u, 255u,
                              255u, 255u, 255u, 255u, 0u, 0u, 0u, 0u };
        ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
        ITextureResource::Size size = {};
        size.width = 2;
        size.height = 2;
        size.depth = 1;

        ITextureResource::Desc textureDesc = {};
        textureDesc.type = IResource::Type::Texture2D;
        textureDesc.numMipLevels = 1;
        textureDesc.arraySize = 1;
        textureDesc.size = size;
        textureDesc.defaultState = ResourceState::ShaderResource;
        textureDesc.format = gfx::Format::R8G8B8A8_UNORM;
        textureDesc.isShared = true;

        ComPtr<ITextureResource> srcTexture;
        GFX_CHECK_CALL_ABORT(srcDevice->createTextureResource(
            textureDesc,
            &subData,
            srcTexture.writeRef()));

        InteropHandle sharedHandle;
        GFX_CHECK_CALL_ABORT(srcTexture->getSharedHandle(&sharedHandle));
        ComPtr<ITextureResource> dstTexture;
        size_t sizeInBytes = 0;
        size_t alignment = 0;
        GFX_CHECK_CALL_ABORT(srcDevice->getTextureAllocationInfo(textureDesc, &sizeInBytes, &alignment));
        GFX_CHECK_CALL_ABORT(dstDevice->createTextureFromSharedHandle(sharedHandle, textureDesc, sizeInBytes, dstTexture.writeRef()));
        // Reading back the buffer from srcDevice to make sure it's been filled in before reading anything back from dstDevice
        // TODO: Implement actual synchronization (and not this hacky solution)
        compareComputeResult(
            dstDevice,
            dstTexture,
            ResourceState::ShaderResource,
            texData,
            sizeof(texData));

//         ComPtr<IResourceView> texView;
//         IResourceView::Desc texViewDesc = {};
//         texViewDesc.type = IResourceView::Type::ShaderResource;
//         texViewDesc.format = textureDesc.format;
//         GFX_CHECK_CALL_ABORT(dstDevice->createTextureView(dstTexture, texViewDesc, texView.writeRef()));
// 
//         IBufferResource::Desc bufferDesc = {};
//         float initialData[16] = { 0.0f };
//         bufferDesc.sizeInBytes = 16 * sizeof(float);
//         bufferDesc.format = gfx::Format::Unknown;
//         bufferDesc.elementSize = sizeof(float);
//         bufferDesc.allowedStates = ResourceStateSet(
//             ResourceState::ShaderResource,
//             ResourceState::UnorderedAccess,
//             ResourceState::CopyDestination,
//             ResourceState::CopySource);
//         bufferDesc.defaultState = ResourceState::UnorderedAccess;
//         bufferDesc.cpuAccessFlags = AccessFlag::Write | AccessFlag::Read;
// 
//         ComPtr<IBufferResource> outBuffer;
//         GFX_CHECK_CALL_ABORT(dstDevice->createBufferResource(
//             bufferDesc,
//             initialData,
//             outBuffer.writeRef()));
// 
//         ComPtr<IResourceView> bufferView;
//         IResourceView::Desc viewDesc = {};
//         viewDesc.type = IResourceView::Type::UnorderedAccess;
//         viewDesc.format = Format::Unknown;
//         GFX_CHECK_CALL_ABORT(dstDevice->createBufferView(outBuffer, viewDesc, bufferView.writeRef()));
// 
//         Slang::ComPtr<ITransientResourceHeap> transientHeap;
//         ITransientResourceHeap::Desc transientHeapDesc = {};
//         transientHeapDesc.constantBufferSize = 4096;
//         GFX_CHECK_CALL_ABORT(
//             dstDevice->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));
// 
//         ComPtr<IShaderProgram> shaderProgram;
//         slang::ProgramLayout* slangReflection;
//         GFX_CHECK_CALL_ABORT(loadComputeProgram(dstDevice, shaderProgram, "trivial-copy", "copyTexFloat4", slangReflection));
// 
//         ComputePipelineStateDesc pipelineDesc = {};
//         pipelineDesc.program = shaderProgram.get();
//         ComPtr<gfx::IPipelineState> pipelineState;
//         GFX_CHECK_CALL_ABORT(
//             dstDevice->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
// 
//         {
//             ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
//             auto queue = dstDevice->createCommandQueue(queueDesc);
// 
//             auto commandBuffer = transientHeap->createCommandBuffer();
//             auto encoder = commandBuffer->encodeComputeCommands();
// 
//             auto rootObject = encoder->bindPipeline(pipelineState);
// 
//             ShaderCursor rootCursor(rootObject); // get a cursor the the first entry-point.
// 
//             // Bind texture view to the entry point
//             rootCursor.getPath("tex").setResource(texView);
// 
//             // Bind buffer view to the entry point.
//             rootCursor.getPath("buffer").setResource(bufferView);
// 
//             encoder->dispatchCompute(1, 1, 1);
//             encoder->endEncoding();
//             commandBuffer->close();
//             queue->executeCommandBuffer(commandBuffer);
//             queue->wait();
//         }
// 
//         compareComputeResult(
//             dstDevice,
//             outBuffer,
//             Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.498039216f, 1.0f,
//                                     1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    }

    void sharedTextureTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum srcApi, Slang::RenderApiFlag::Enum dstApi)
    {
        auto srcDevice = createTestingDevice(context, srcApi);
        auto dstDevice = createTestingDevice(context, dstApi);
        if (!srcDevice || !dstDevice)
        {
            SLANG_IGNORE_TEST;
        }

        sharedTextureTestImpl(srcDevice, dstDevice, context);
    }
#if SLANG_WIN64
    SLANG_UNIT_TEST(sharedTextureD3D12ToCUDA)
    {
        sharedTextureTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12, Slang::RenderApiFlag::CUDA);
    }

#if SLANG_WINDOWS_FAMILY // TODO: Remove when Linux support is added
    SLANG_UNIT_TEST(sharedTextureVulkanToCUDA)
    {
        sharedTextureTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan, Slang::RenderApiFlag::CUDA);
    }
#endif
#endif
}
