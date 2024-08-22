#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"
#include "source/core/slang-blob.h"
#include "source/core/slang-memory-file-system.h"
#include "source/core/slang-io.h"

using namespace gfx;

namespace gfx_test
{
    // Test that mixing precompiled and non-precompiled modules is working.

    static Slang::Result precompileProgram(
        gfx::IDevice* device,
        ISlangMutableFileSystem* fileSys,
        const char* shaderModuleName,
        bool precompileToTarget)
    {
        Slang::ComPtr<slang::ISession> slangSession;
        SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
        slang::SessionDesc sessionDesc = {};
        auto searchPaths = getSlangSearchPaths();
        sessionDesc.searchPathCount = searchPaths.getCount();
        sessionDesc.searchPaths = searchPaths.getBuffer();
        auto globalSession = slangSession->getGlobalSession();
        globalSession->createSession(sessionDesc, slangSession.writeRef());

        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slang::IModule* module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        if (precompileToTarget)
        {
            SlangCompileTarget target;
            switch (device->getDeviceInfo().deviceType)
            {
            case gfx::DeviceType::DirectX12:
                target = SLANG_DXIL;
                break;
            default:
                return SLANG_FAIL;
            }
            module->precompileForTarget(target, diagnosticsBlob.writeRef());
        }

        // Write loaded modules to memory file system.
        for (SlangInt i = 0; i < slangSession->getLoadedModuleCount(); i++)
        {
            auto module = slangSession->getLoadedModule(i);
            auto path = module->getFilePath();
            if (path)
            {
                auto name = module->getName();
                ComPtr<ISlangBlob> outBlob;
                module->serialize(outBlob.writeRef());
                fileSys->saveFileBlob((Slang::String(name) + ".slang-module").getBuffer(), outBlob);
            }
        }
        return SLANG_OK;
    }

    void precompiledModule2TestImplCommon(IDevice* device, UnitTestContext* context, bool precompileToTarget)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        // First, load and compile the slang source.
        ComPtr<ISlangMutableFileSystem> memoryFileSystem = ComPtr<ISlangMutableFileSystem>(new Slang::MemoryFileSystem());

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(precompileProgram(device, memoryFileSystem.get(), "precompiled-module-imported", precompileToTarget));

        // Next, load the precompiled slang program.
        Slang::ComPtr<slang::ISession> slangSession;
        device->getSlangSession(slangSession.writeRef());
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        slang::TargetDesc targetDesc = {};
        switch (device->getDeviceInfo().deviceType)
        {
        case gfx::DeviceType::DirectX12:
            targetDesc.format = SLANG_DXIL;
            targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("sm_6_1");
            break;
        case gfx::DeviceType::Vulkan:
            targetDesc.format = SLANG_SPIRV;
            targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("GLSL_460");
            break;
        }
        sessionDesc.targets = &targetDesc;
        sessionDesc.fileSystem = memoryFileSystem.get();
        auto globalSession = slangSession->getGlobalSession();
        globalSession->createSession(sessionDesc, slangSession.writeRef());

        const char* moduleSrc = R"(
            import "precompiled-module-imported";

            // Main entry-point. 

            using namespace ns;

            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
                uint3 sv_dispatchThreadID : SV_DispatchThreadID,
                uniform RWStructuredBuffer <float> buffer)
            {
                buffer[sv_dispatchThreadID.x] = helperFunc() + helperFunc1();
            }
        )";
        memoryFileSystem->saveFile("precompiled-module.slang", moduleSrc, strlen(moduleSrc));
        GFX_CHECK_CALL_ABORT(loadComputeProgram(device, slangSession, shaderProgram, "precompiled-module", "computeMain", slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

        const int numberCount = 4;
        float initialData[] = { 0.0f, 0.0f, 0.0f, 0.0f };
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

        ComPtr<IResourceView> bufferView;
        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = Format::Unknown;
        GFX_CHECK_CALL_ABORT(
            device->createBufferView(numbersBuffer, nullptr, viewDesc, bufferView.writeRef()));

        // We have done all the set up work, now it is time to start recording a command buffer for
        // GPU execution.
        {
            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor entryPointCursor(
                rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
            // Bind buffer view to the entry point.
            entryPointCursor.getPath("buffer").setResource(bufferView);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        compareComputeResult(
            device,
            numbersBuffer,
            Slang::makeArray<float>(3.0f, 3.0f, 3.0f, 3.0f));
    }

    void precompiledModule2TestImpl(IDevice* device, UnitTestContext* context)
    {
        precompiledModule2TestImplCommon(device, context, false);
    }

    void precompiledTargetModule2TestImpl(IDevice* device, UnitTestContext* context)
    {
        precompiledModule2TestImplCommon(device, context, true);
    }

    SLANG_UNIT_TEST(precompiledModule2D3D12)
    {
        runTestImpl(precompiledModule2TestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(precompiledTargetModule2D3D12)
    {
        runTestImpl(precompiledTargetModule2TestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(precompiledModule2Vulkan)
    {
        runTestImpl(precompiledModule2TestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
