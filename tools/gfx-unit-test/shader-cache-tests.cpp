#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#include "source/core/slang-memory-file-system.h"
#include "source/core/slang-file-system.h"

using namespace gfx;

namespace gfx_test
{

    struct BaseShaderCacheTest
    {
        UnitTestContext* context;
        Slang::RenderApiFlag::Enum api;

        ComPtr<IDevice> device;
        ComPtr<IPipelineState> pipelineState;
        ComPtr<IResourceView> bufferView;

        IDevice::ShaderCacheDesc shaderCache = {};

        // Two file systems in order to get around problems posed by the testing framework.
        // 
        // - diskFileSystem - Used to save any files that must exist on disk for subsequent
        //                    save/load function calls (most prominently loadComputeProgram()) to pick up.
        // - cacheFileSystem - Used to hold for the actual cache for all tests. This removes the need to
        //                     manually clean out old cache files from previous test runs as it is
        //                     located in-memory. This is the file system passed to device creation
        //                     as part of the shader cache desc.
        ComPtr<ISlangMutableFileSystem> diskFileSystem;
        ComPtr<ISlangMutableFileSystem> cacheFileSystem;

        // Simple compute shaders we can pipe to our individual shader files for cache testing
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

        void freeOldResources()
        {
            bufferView = nullptr;
            pipelineState = nullptr;
            device = nullptr;
        }

        // TODO: This should be removed at some point. Currently exists as a workaround for module loading
        // seemingly not accounting for updated shader code under the same module name with the same entry point.
        void generateNewDevice()
        {
            freeOldResources();
            device = createTestingDevice(context, api, shaderCache);
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

            cacheFileSystem = new Slang::MemoryFileSystem();
            diskFileSystem = Slang::OSFileSystem::getMutableSingleton();
            diskFileSystem = new Slang::RelativeFileSystem(diskFileSystem, "tools/gfx-unit-test");

            shaderCache.shaderCacheFileSystem = cacheFileSystem;
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
    };

    // One shader file on disk, all modifications are done to the same file
    struct SingleEntryShaderCache : BaseShaderCacheTest
    {
        void generateNewPipelineState(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader.slang", shaderContents.getBuffer(), shaderContents.getLength());

            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "shader-cache-shader", "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(contentsA);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(contentsA);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(contentsC);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);
        }
    };

    // Several shader files on disk, modifications may be done to any file
    struct MultipleEntryShaderCache : BaseShaderCacheTest
    {
        void modifyShaderA(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-A.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void modifyShaderB(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-B.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void modifyShaderC(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-C.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void generateNewPipelineState(GfxIndex shaderIndex)
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            char* shaderFilename;
            switch (shaderIndex)
            {
            case 0:
                shaderFilename = "shader-cache-shader-A";
                break;
            case 1:
                shaderFilename = "shader-cache-shader-B";
                break;
            case 2:
                shaderFilename = "shader-cache-shader-C";
                break;
            default:
                // Should never reach this point since we wrote the test
                SLANG_IGNORE_TEST;
            }
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, shaderFilename, "computeMain", slangReflection));
            
            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void checkAllCacheEntries()
        {
            generateNewPipelineState(0);
            submitGPUWork();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(2);
            submitGPUWork();
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            generateNewDevice();
            createRequiredResources();
            modifyShaderA(contentsA);
            modifyShaderB(contentsB);
            modifyShaderC(contentsC);
            checkAllCacheEntries();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 3);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            checkAllCacheEntries();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 3);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            modifyShaderA(contentsB);
            checkAllCacheEntries();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 2);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);

            generateNewDevice();
            createRequiredResources();
            modifyShaderA(contentsC);
            modifyShaderB(contentsA);
            modifyShaderC(contentsB);
            checkAllCacheEntries();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 3);
        }
    };

    // One shader file on disk containing several entry points, no modifications are made to the file
    struct MultipleEntryPointShader : BaseShaderCacheTest
    {
        void generateNewPipelineState(GfxIndex shaderIndex)
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            char* entryPointName;
            switch (shaderIndex)
            {
            case 0:
                entryPointName = "computeA";
                break;
            case 1:
                entryPointName = "computeB";
                break;
            case 2:
                entryPointName = "computeC";
                break;
            default:
                // Should never reach this point since we wrote the test
                SLANG_IGNORE_TEST;
            }
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "multiple-entry-point-shader-cache-shader", entryPointName, slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(0);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(0);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(2);
            submitGPUWork();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(0);
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 2);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);
        }
    };

    // One shader file contains an import/include, direct code modifications are made to the imported file
    // This test specifically checks four cases:
    //    1. import w/o changes in the imported file
    //    2. import w/ changes in the imported file
    //    3. #include w/o changes in the included file (the included file is the same as the imported file in the prior step)
    //    4. #include w/ changes in the included file
    struct ShaderFileImportsShaderCache : BaseShaderCacheTest
    {
        Slang::String importedContentsA = Slang::String(
            R"(struct TestFunction
            {
                void simpleElementAdd(RWStructuredBuffer<float> buffer, uint index)
                {
                    var input = buffer[index];
                    buffer[index] = input + 1.0f;
                }
            };)");

        Slang::String importedContentsB = Slang::String(
            R"(struct TestFunction
            {
                void simpleElementAdd(RWStructuredBuffer<float> buffer, uint index)
                {
                    var input = buffer[index];
                    buffer[index] = input + 2.0f;
                }
            };)");

        Slang::String importFile = Slang::String(
            R"(import imported;

            uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                TestFunction test;
                for (uint i = 0; i < 4; ++i)
                {
                    test.simpleElementAdd(buffer, i);
                }
            })");

        Slang::String includeFile = Slang::String(
            R"(#include "imported.slang"

            uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                TestFunction test;
                for (uint i = 0; i < 4; ++i)
                {
                    test.simpleElementAdd(buffer, i);
                }
            })");

        void initializeFiles()
        {
            diskFileSystem->saveFile("imported.slang", importedContentsA.getBuffer(), importedContentsA.getLength());
            diskFileSystem->saveFile("importing-shader-cache-shader.slang", importFile.getBuffer(), importFile.getLength());
        }

        void modifyImportedFile(Slang::String importedContents)
        {
            diskFileSystem->saveFile("imported.slang", importedContents.getBuffer(), importedContents.getLength());
        }

        void changeImportToInclude()
        {
            diskFileSystem->saveFile("importing-shader-cache-shader.slang", includeFile.getBuffer(), includeFile.getLength());
        }

        void generateNewPipelineState()
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "importing-shader-cache-shader", "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            generateNewDevice();
            createRequiredResources();
            initializeFiles();
            generateNewPipelineState();
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            modifyImportedFile(importedContentsB);
            generateNewPipelineState();
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);

            generateNewDevice();
            createRequiredResources();
            changeImportToInclude();
            generateNewPipelineState();
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);

            generateNewDevice();
            createRequiredResources();
            modifyImportedFile(importedContentsA);
            generateNewPipelineState();
            submitGPUWork();

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);
        }
    };

    // One shader featuring multiple kinds of shader objects that can be bound.
    struct SpecializationArgsEntries : BaseShaderCacheTest
    {
        slang::ProgramLayout* slangReflection;

        void createAddTransformer(IShaderObject** transformer)
        {
            slang::TypeReflection* addTransformerType =
                slangReflection->findTypeByName("AddTransformer");
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                addTransformerType, ShaderObjectContainerType::None, transformer));

            float c = 1.0f;
            ShaderCursor(*transformer).getPath("c").setData(&c, sizeof(float));
        }

        void createMulTransformer(IShaderObject** transformer)
        {
            slang::TypeReflection* mulTransformerType =
                slangReflection->findTypeByName("MulTransformer");
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                mulTransformerType, ShaderObjectContainerType::None, transformer));

            float c = 1.0f;
            ShaderCursor(*transformer).getPath("c").setData(&c, sizeof(float));
        }

        void submitGPUWork(GfxIndex transformerType)
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

            ComPtr<IShaderObject> transformer;
            switch (transformerType)
            {
            case 0:
                createAddTransformer(transformer.writeRef());
                break;
            case 1:
                createMulTransformer(transformer.writeRef());
                break;
            default:
                /* Should not get here */
                SLANG_IGNORE_TEST;
            }

            ShaderCursor entryPointCursor(rootObject->getEntryPoint(0));
            entryPointCursor.getPath("buffer").setResource(bufferView);

            entryPointCursor.getPath("transformer").setObject(transformer);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void generateNewPipelineState()
        {
            ComPtr<IShaderProgram> shaderProgram;

            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "compute-smoke", "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState();
            submitGPUWork(0);

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState();
            submitGPUWork(1);

            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);
        }
    };

    // Same as MultipleEntryShaderCache, but we now have a maximum cache entry limit of 2, so the cache
// will now evict entries when it reaches capacity
    struct CacheWithMaxEntryLimit : BaseShaderCacheTest
    {
        void modifyShaderA(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-A.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void modifyShaderB(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-B.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void modifyShaderC(Slang::String shaderContents)
        {
            diskFileSystem->saveFile("shader-cache-shader-C.slang", shaderContents.getBuffer(), shaderContents.getLength());
        }

        void generateNewPipelineState(GfxIndex shaderIndex)
        {
            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            char* shaderFilename;
            switch (shaderIndex)
            {
            case 0:
                shaderFilename = "shader-cache-shader-A";
                break;
            case 1:
                shaderFilename = "shader-cache-shader-B";
                break;
            case 2:
                shaderFilename = "shader-cache-shader-C";
                break;
            default:
                // Should never reach this point since we wrote the test
                SLANG_IGNORE_TEST;
            }
            GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, shaderFilename, "computeMain", slangReflection));

            ComputePipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            GFX_CHECK_CALL_ABORT(
                device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));
        }

        void run()
        {
            ComPtr<IShaderCacheStatistics> shaderCacheStats;

            // Due to needing a workaround to prevent loading old, outdated modules, we need to
            // recreate the device between each segment of the test. However, we need to maintain the
            // same cache filesystem for the duration of the test, so the device is immediately recreated
            // to ensure we can pass the filesystem all the way through.
            //
            // TODO: Remove the repeated generateNewDevice() and createRequiredResources() calls once
            // a solution exists that allows source code changes under the same module name to be picked
            // up on load.
            shaderCache.entryCountLimit = 2;
            generateNewDevice();
            createRequiredResources();
            modifyShaderA(contentsA);
            modifyShaderB(contentsB);
            modifyShaderC(contentsC);
            generateNewPipelineState(0);
            submitGPUWork();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(2);
            submitGPUWork();

            // Cache limit 2, three unique shaders
            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 3);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(0);
            submitGPUWork();

            // Cache limit 2, access shaders B and then A
            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);

            generateNewDevice();
            createRequiredResources();
            modifyShaderB(contentsA);
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(2);
            submitGPUWork();

            // Cache limit 2, access shaders B and then C after modifying B
            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);

            shaderCache.entryCountLimit = 3;
            generateNewDevice();
            createRequiredResources();
            modifyShaderA(contentsC);
            modifyShaderC(contentsB);
            generateNewPipelineState(0);
            submitGPUWork();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(2);
            submitGPUWork();

            // Cache limit 3, access shaders A then B then C after modifying A and C
            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 1);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 1);

            generateNewDevice();
            createRequiredResources();
            generateNewPipelineState(0);
            submitGPUWork();
            generateNewPipelineState(1);
            submitGPUWork();
            generateNewPipelineState(2);
            submitGPUWork();

            // Cache limit 3, access shaders A then B then C
            device->queryInterface(SLANG_UUID_IShaderCacheStatistics, (void**)shaderCacheStats.writeRef());
            SLANG_CHECK(shaderCacheStats->getCacheMissCount() == 0);
            SLANG_CHECK(shaderCacheStats->getCacheHitCount() == 3);
            SLANG_CHECK(shaderCacheStats->getCacheEntryDirtyCount() == 0);
        }
    };

    template <typename T>
    void shaderCacheTestImpl(ComPtr<IDevice> device, UnitTestContext* context)
    {
        T test;
        test.init(device, context);
        test.run();
    }

    SLANG_UNIT_TEST(singleEntryShaderCacheD3D12)
    {
        runTestImpl(shaderCacheTestImpl<SingleEntryShaderCache>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(singleEntryShaderCacheVulkan)
    {
        runTestImpl(shaderCacheTestImpl<SingleEntryShaderCache>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(multipleEntryShaderCacheD3D12)
    {
        runTestImpl(shaderCacheTestImpl<MultipleEntryShaderCache>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(multipleEntryShaderCacheVulkan)
    {
        runTestImpl(shaderCacheTestImpl<MultipleEntryShaderCache>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(multipleEntryPointShaderCacheD3D12)
    {
        runTestImpl(shaderCacheTestImpl<MultipleEntryPointShader>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(multipleEntryPointShaderCacheVulkan)
    {
        runTestImpl(shaderCacheTestImpl<MultipleEntryPointShader>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(shaderFileImportsShaderCacheD3D12)
    {
        runTestImpl(shaderCacheTestImpl<ShaderFileImportsShaderCache>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(shaderFileImportsShaderCacheVulkan)
    {
        runTestImpl(shaderCacheTestImpl<ShaderFileImportsShaderCache>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(specializationArgsShaderCacheD3D12)
    {
        runTestImpl(shaderCacheTestImpl<SpecializationArgsEntries>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(specializationArgsShaderCacheVulkan)
    {
        runTestImpl(shaderCacheTestImpl<SpecializationArgsEntries>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(cacheEvictionPolicyD3D12)
    {
        runTestImpl(shaderCacheTestImpl<CacheWithMaxEntryLimit>, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(cacheEvictionPolicyVulkan)
    {
        runTestImpl(shaderCacheTestImpl<CacheWithMaxEntryLimit>, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
}
