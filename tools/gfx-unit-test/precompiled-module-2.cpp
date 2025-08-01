#if 0
// Duplicated: This test is identical to slang-rhi\tests\test-precompiled-module-cache.cpp
// TODO_TESTING port

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
// Test that mixing precompiled and non-precompiled modules is working.

static Slang::Result precompileProgram(
    rhi::IDevice* device,
    ISlangMutableFileSystem* fileSys,
    const char* shaderModuleName,
    PrecompilationMode precompilationMode)
{
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    slang::SessionDesc sessionDesc = {};
    auto searchPaths = getSlangSearchPaths();
    sessionDesc.searchPathCount = searchPaths.getCount();
    sessionDesc.searchPaths = searchPaths.getBuffer();
    auto globalSession = slangSession->getGlobalSession();
    globalSession->createSession(sessionDesc, slangSession.writeRef());

    slang::IModule* module;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
    }
    if (!module)
        return SLANG_FAIL;

    if (precompilationMode == PrecompilationMode::InternalLink ||
        precompilationMode == PrecompilationMode::ExternalLink)
    {
        SlangCompileTarget target;
        switch (device->getInfo().deviceType)
        {
        case rhi::DeviceType::D3D12:
            target = SLANG_DXIL;
            break;
        case rhi::DeviceType::Vulkan:
            target = SLANG_SPIRV;
            break;
        default:
            return SLANG_FAIL;
        }

        ComPtr<slang::IModulePrecompileService_Experimental> precompileService;
        if (module->queryInterface(
                slang::SLANG_UUID_IModulePrecompileService_Experimental,
                (void**)precompileService.writeRef()) == SLANG_OK)
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            auto res = precompileService->precompileForTarget(target, diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
            SLANG_RETURN_ON_FAIL(res);

            // compile a second time to check for driver bugs.
            diagnosticsBlob = nullptr;
            res = precompileService->precompileForTarget(target, diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
            SLANG_RETURN_ON_FAIL(res);
        }
    }

    // Write loaded modules to file system.
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

void precompiledModule2TestImplCommon(
    IDevice* device,
    UnitTestContext* context,
    PrecompilationMode precompilationMode)
{
    // First, load and compile the slang source.
    ComPtr<ISlangMutableFileSystem> memoryFileSystem =
        ComPtr<ISlangMutableFileSystem>(new Slang::MemoryFileSystem());

    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(precompileProgram(
        device,
        memoryFileSystem.get(),
        "precompiled-module-imported",
        precompilationMode));

    // Next, load the precompiled slang program.
    Slang::ComPtr<slang::ISession> slangSession;
    device->getSlangSession(slangSession.writeRef());
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    switch (device->getInfo().deviceType)
    {
    case rhi::DeviceType::D3D12:
        targetDesc.format = SLANG_DXIL;
        targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("sm_6_6");
        break;
    case rhi::DeviceType::Vulkan:
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("GLSL_460");
        break;
    }
    sessionDesc.targets = &targetDesc;
    sessionDesc.fileSystem = memoryFileSystem.get();

    Slang::List<slang::CompilerOptionEntry> options;
    slang::CompilerOptionEntry skipDownstreamLinkingOption;
    skipDownstreamLinkingOption.name = slang::CompilerOptionName::SkipDownstreamLinking;
    skipDownstreamLinkingOption.value.kind = slang::CompilerOptionValueKind::Int;
    skipDownstreamLinkingOption.value.intValue0 =
        precompilationMode == PrecompilationMode::ExternalLink;
    options.add(skipDownstreamLinkingOption);

    sessionDesc.compilerOptionEntries = options.getBuffer();
    sessionDesc.compilerOptionEntryCount = options.getCount();
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
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        slangSession,
        shaderProgram,
        "precompiled-module",
        "computeMain",
        slangReflection,
        precompilationMode));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<rhi::IComputePipeline> pipeline = device->createComputePipeline(pipelineDesc);
    const int numberCount = 4;
    float initialData[] = {0.0f, 0.0f, 0.0f, 0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);

        auto commandBuffer = queue->createCommandEncoder();
        auto encoder = commandBuffer->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipeline);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer view to the entry point.
        entryPointCursor.getPath("buffer").setBinding(numbersBuffer);

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandBuffer->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{3.0f, 3.0f, 3.0f, 3.0f});
}

void precompiledModule2TestImpl(IDevice* device, UnitTestContext* context)
{
    precompiledModule2TestImplCommon(device, context, PrecompilationMode::SlangIR);
}

void precompiledTargetModule2InternalLinkTestImpl(IDevice* device, UnitTestContext* context)
{
    precompiledModule2TestImplCommon(device, context, PrecompilationMode::InternalLink);
}

void precompiledTargetModule2ExternalLinkTestImpl(IDevice* device, UnitTestContext* context)
{
    precompiledModule2TestImplCommon(device, context, PrecompilationMode::ExternalLink);
}

SLANG_UNIT_TEST(precompiledModule2D3D12)
{
    runTestImpl(precompiledModule2TestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(precompiledTargetModuleInternalLink2D3D12)
{
    runTestImpl(
        precompiledTargetModule2InternalLinkTestImpl,
        unitTestContext,
        DeviceType::D3D12);
}

/*
// Unavailable on D3D12/DXIL currently
SLANG_UNIT_TEST(precompiledTargetModuleExternalLink2D3D12)
{
    runTestImpl(precompiledTargetModule2ExternalLinkTestImpl, unitTestContext,
DeviceType::D3D12);
}
*/

SLANG_UNIT_TEST(precompiledModule2Vulkan)
{
    runTestImpl(precompiledModule2TestImpl, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(precompiledTargetModule2InternalLinkVulkan)
{
    runTestImpl(
        precompiledTargetModule2InternalLinkTestImpl,
        unitTestContext,
        DeviceType::Vulkan);
}

SLANG_UNIT_TEST(precompiledTargetModule2ExternalLinkVulkan)
{
    runTestImpl(
        precompiledTargetModule2ExternalLinkTestImpl,
        unitTestContext,
        DeviceType::Vulkan);
}

} // namespace gfx_test

#endif
