#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
static Slang::Result loadProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection,
    const char* additionalModuleSource)
{
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return SLANG_FAIL;

    auto additionalModuleBlob =
        Slang::UnownedRawBlob::create(additionalModuleSource, strlen(additionalModuleSource));
    slang::IModule* additionalModule =
        slangSession->loadModuleFromSource("linkedConstants", "path", additionalModuleBlob);

    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module->findEntryPointByName(entryPointName, computeEntryPoint.writeRef()));

    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(module);
    componentTypes.add(computeEntryPoint);
    componentTypes.add(additionalModule);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    composedProgram = linkedProgram;
    slangReflection = composedProgram->getLayout();

    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = composedProgram.get();

    auto shaderProgram = device->createShaderProgram(programDesc);

    outShaderProgram = shaderProgram;
    return SLANG_OK;
}

void linkTimeConstantTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        shaderProgram,
        "link-time-constant",
        "computeMain",
        slangReflection,
        R"(
                export static const bool turnOnFeature = true;
                export static const float constValue = 2.0;
                export static const uint numthread = 2;
                export static const int arraySize = 4;
            )"));

    SlangUInt threadGroupSizes[3];
    slangReflection->findEntryPointByName("computeMain")
        ->getComputeThreadGroupSize(3, threadGroupSizes);
    SLANG_CHECK(threadGroupSizes[0] == 2 && threadGroupSizes[1] == 1 && threadGroupSizes[2] == 1);

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 0.0f, 0.0f, 0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
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
        auto encoder = commandEncoder->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipelineState);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(numbersBuffer);

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{2.0f});
}

SLANG_UNIT_TEST(linkTimeConstantD3D12)
{
    runTestImpl(linkTimeConstantTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeConstantVulkan)
{
    runTestImpl(linkTimeConstantTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
