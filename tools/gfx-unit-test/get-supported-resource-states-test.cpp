#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
using namespace gfx;

namespace
{
    using namespace gfx_test;

    struct GetSupportedResourceStatesBase
    {
        IDevice* device;
        UnitTestContext* context;

        ResourceStateSet formatSupportedStates;
        ResourceStateSet textureAllowedStates;
        ResourceStateSet bufferAllowedStates;

        ComPtr<ITextureResource> texture;
        ComPtr<IBufferResource> buffer;

        void init(IDevice* device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
        }

        void transitionResourceStates(IDevice* device)
        {
            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeResourceCommands();
            ResourceState currentTextureState = texture->getDesc()->defaultState;
            ResourceState currentBufferState = buffer->getDesc()->defaultState;

            for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; ++i)
            {
                auto nextState = (ResourceState)i;
                if (formatSupportedStates.contains(nextState))
                {
                    if (bufferAllowedStates.contains(nextState))
                    {
                        encoder->bufferBarrier(buffer, currentBufferState, nextState);
                        currentBufferState = nextState;
                    }
                    if (textureAllowedStates.contains(nextState))
                    {
                        encoder->textureBarrier(texture, currentTextureState, nextState);
                        currentTextureState = nextState;
                    }
                }
            }
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            Format format = Format::R32G32B32A32_FLOAT;
            GFX_CHECK_CALL_ABORT(device->getFormatSupportedResourceStates(format, &formatSupportedStates));

            textureAllowedStates.add(
                ResourceState::RenderTarget,
                ResourceState::DepthRead,
                ResourceState::DepthWrite,
                ResourceState::Present,
                ResourceState::ResolveSource,
                ResourceState::ResolveDestination,
                ResourceState::Undefined,
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopySource,
                ResourceState::CopyDestination);

            bufferAllowedStates.add(
                ResourceState::VertexBuffer,
                ResourceState::IndexBuffer,
                ResourceState::ConstantBuffer,
                ResourceState::StreamOutput,
                ResourceState::IndirectArgument,
                ResourceState::AccelerationStructure,
                ResourceState::Undefined,
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopySource,
                ResourceState::CopyDestination);

            ResourceState currentState = ResourceState::UnorderedAccess;
            ITextureResource::Size extent;
            extent.width = 2;
            extent.height = 2;
            extent.depth = 1;

            float initialData[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
            ITextureResource::SubresourceData subdata = { (void*)initialData, 32, 0 };

            ITextureResource::Desc texDesc = {};
            texDesc.type = IResource::Type::Texture2D;
            texDesc.numMipLevels = 1;
            texDesc.arraySize = 1;
            texDesc.size = extent;
            texDesc.defaultState = currentState;
            texDesc.allowedStates = formatSupportedStates & textureAllowedStates;
            texDesc.memoryType = MemoryType::DeviceLocal;
            texDesc.format = format;

            GFX_CHECK_CALL_ABORT(device->createTextureResource(
                texDesc,
                nullptr,
                texture.writeRef()));

            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = 256;
            bufferDesc.format = gfx::Format::Unknown;
            bufferDesc.elementSize = sizeof(float);
            bufferDesc.allowedStates = formatSupportedStates & bufferAllowedStates;
            bufferDesc.defaultState = currentState;
            bufferDesc.memoryType = MemoryType::DeviceLocal;
            
            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                nullptr,
                buffer.writeRef()));

            transitionResourceStates(device);
        }
    };

    void supportedResourceStatesTestImpl(IDevice* device, UnitTestContext* context)
    {
        GetSupportedResourceStatesBase test;
        test.init(device, context);
        test.run();
    }
}

namespace gfx_test
{
    SLANG_UNIT_TEST(getSupportedResourceStatesD3D12)
    {
        runTestImpl(supportedResourceStatesTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(getSupportedResourceStatesVulkan)
    {
        runTestImpl(supportedResourceStatesTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
