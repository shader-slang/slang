#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-memory-file-system.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
static Slang::Result precompileProgram(
    IDevice* device,
    ISlangMutableFileSystem* fileSys,
    const char* shaderModuleName)
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

void precompiledModuleTestImpl(IDevice* device, UnitTestContext* context)
{
    // First, load and compile the slang source.
    ComPtr<ISlangMutableFileSystem> memoryFileSystem =
        ComPtr<ISlangMutableFileSystem>(new Slang::MemoryFileSystem());

    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(precompileProgram(device, memoryFileSystem.get(), "precompiled-module"));

    // Next, load the precompiled slang program.
    Slang::ComPtr<slang::ISession> slangSession;
    device->getSlangSession(slangSession.writeRef());
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    slang::TargetDesc targetDesc = {};
    switch (device->getInfo().deviceType)
    {
    case DeviceType::D3D12:
        targetDesc.format = SLANG_DXIL;
        targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("sm_6_1");
        break;
    case DeviceType::Vulkan:
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = device->getSlangSession()->getGlobalSession()->findProfile("GLSL_460");
        break;
    }
    sessionDesc.targets = &targetDesc;
    sessionDesc.fileSystem = memoryFileSystem.get();
    auto globalSession = slangSession->getGlobalSession();
    globalSession->createSession(sessionDesc, slangSession.writeRef());
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        slangSession,
        shaderProgram,
        "precompiled-module",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 0.0f, 0.0f, 0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor entryPointCursor(
                rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
            // Bind buffer directly to the entry point.
            entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{3.0f, 3.0f, 3.0f, 3.0f});
}

SLANG_UNIT_TEST(precompiledModuleD3D12)
{
    runTestImpl(precompiledModuleTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(precompiledModuleVulkan)
{
    runTestImpl(precompiledModuleTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
