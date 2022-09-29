#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"
#include "source/core/slang-riff-file-system.h"

using namespace gfx;

namespace gfx_test
{

    struct ShaderCacheTest
    {
        ComPtr<IDevice> device;
        UnitTestContext* context;
        Slang::RenderApiFlag::Enum api;

        ComPtr<Slang::RiffFileSystem> fileSystem;
        //ComPtr<ISlangMutableFileSystem> fileSystem;
        ComPtr<IPipelineState> pipelineState;
        ComPtr<IResourceView> bufferView;

        ComPtr<IShaderCacheStatistics> shaderCacheStats;

        Slang::String contentsA = Slang::String(
            R"(uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 1.0f;
            })");    

        Slang::String contentsB = Slang::String(
            R"(uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 2.0f;
            })");

        Slang::String contentsC = Slang::String(
            R"(uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                var input = buffer[sv_dispatchThreadID.x];
                buffer[sv_dispatchThreadID.x] = input + 3.0f;
            })");

        void createRequiredResources()
        {
            const int numberCount = 4;
            float initialData[] = { 0.0f, 1.0f, 2.0f, 3.0f };
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.sizeInBytes = numberCount * sizeof(float);
            bufferDesc.format = gfx::Format::Unknown;
            bufferDesc.elementSize = sizeof(float);
            bufferDesc.allowedStates = ResourceStateSet(
                ResourceState::ShaderResource,
                ResourceState::UnorderedAccess,
                ResourceState::CopyDestination,
                ResourceState::CopySource);
            bufferDesc.defaultState = ResourceState::UnorderedAccess;
            bufferDesc.memoryType = MemoryType::DeviceLocal;

            ComPtr<IBufferResource> numbersBuffer;
            GFX_CHECK_CALL_ABORT(device->createBufferResource(
                bufferDesc,
                (void*)initialData,
                numbersBuffer.writeRef()));

            IResourceView::Desc viewDesc = {};
            viewDesc.type = IResourceView::Type::UnorderedAccess;
            viewDesc.format = Format::Unknown;
            GFX_CHECK_CALL_ABORT(
                device->createBufferView(numbersBuffer, nullptr, viewDesc, bufferView.writeRef()));
        }

        void generateNewPipelineState(Slang::String shaderContents)
        {
            fileSystem->saveFile("shader-cache-shader.slang", shaderContents.getBuffer(), shaderContents.getLength());

            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "shader-cache-shader", "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        // TODO: This should be removed at some point. Currently exists as a workaround for module loading
        // seemingly not accounting for updated shader code under the same module name with the same entry point.
        void generateNewDevice()
        {
            device = createTestingDevice(context, api, fileSystem);
        }

        void init(ComPtr<IDevice> device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
            switch (device->getDeviceInfo().deviceType)
            {
            case DeviceType::DirectX11:
                api = Slang::RenderApiFlag::D3D11;
                break;
            case DeviceType::DirectX12:
                api = Slang::RenderApiFlag::D3D12;
                break;
            case DeviceType::Vulkan:
                api = Slang::RenderApiFlag::Vulkan;
                break;
            case DeviceType::CPU:
                api = Slang::RenderApiFlag::CPU;
                break;
            case DeviceType::CUDA:
                api = Slang::RenderApiFlag::CUDA;
                break;
            case DeviceType::OpenGl:
                api = Slang::RenderApiFlag::OpenGl;
                break;
            default:
                SLANG_IGNORE_TEST
            }

            fileSystem = new Slang::RiffFileSystem(nullptr);
            //fileSystem = Slang::OSFileSystem::getMutableSingleton();
        }

        void submitGPUWork()
        {
            Slang::ComPtr<ITransientResourceHeap> transientHeap;
            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor rootCursor(rootObject);
            // Bind buffer view to the entry point.
            rootCursor.getPath("buffer").setResource(bufferView);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            createRequiredResources();
            generateNewPipelineState(contentsA);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheEntryMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(contentsA);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheEntryMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(contentsC);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheEntryMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
        }
    };

    void shaderCacheTestImpl(ComPtr<IDevice> device, UnitTestContext* context)
    {
        ShaderCacheTest test;
        test.init(device, context);
        test.run();
    }

    SLANG_UNIT_TEST(shaderCacheD3D12)
    {
        auto fileSystem = Slang::OSFileSystem::getMutableSingleton();
        runTestImpl(shaderCacheTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12, fileSystem);
    }

    SLANG_UNIT_TEST(shaderCacheVulkan)
    {
        auto fileSystem = Slang::OSFileSystem::getMutableSingleton();
        runTestImpl(shaderCacheTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan, fileSystem);
    }

}
