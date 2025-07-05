#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
// In this test,
// we will run a compute shader that compiles to HLSL with a reference to the macro
// "DOWNSTREAM_VALUE" that will be provided to dxc through slang's link-time compiler options. The
// test verifies that `IComponentType2::linkWithOptions()` is able to produce a linked
// IComponentType with additional compiler options. Here we will specify a DownstreamArg compiler
// option to define the value of DOWNSTREAM_VALUE when running dxc.
//
static Slang::Result loadProgram(
    rhi::IDevice* device,
    Slang::ComPtr<rhi::IShaderProgram>& outShaderProgram,
    const char* shaderModuleName,
    const char* entryPointName,
    slang::ProgramLayout*& slangReflection)
{
    Slang::ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> computeEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module->findEntryPointByName(entryPointName, computeEntryPoint.writeRef()));

    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(module);
    componentTypes.add(computeEntryPoint);

    Slang::ComPtr<slang::IComponentType> composedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    slang::CompilerOptionEntry entry;
    entry.name = slang::CompilerOptionName::DownstreamArgs;
    entry.value.kind = slang::CompilerOptionValueKind::String;
    entry.value.stringValue0 = "dxc";
    entry.value.stringValue1 = "-DDOWNSTREAM_VALUE=4.0";
    result = composedProgram
                 ->linkWithOptions(linkedProgram.writeRef(), 1, &entry, diagnosticsBlob.writeRef());
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

void linkTimeOptionTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(
        loadProgram(device, shaderProgram, "link-time-options", "computeMain", slangReflection));

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
        auto computePassEncoder = commandEncoder->beginComputePass();

        auto rootObject = computePassEncoder->bindPipeline(pipelineState);

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        computePassEncoder->dispatchCompute(1, 1, 1);
        computePassEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{4.0f});
}

SLANG_UNIT_TEST(linkTimeOptionD3D12)
{
    runTestImpl(linkTimeOptionTestImpl, unitTestContext, DeviceType::D3D12);
}
} // namespace gfx_test
