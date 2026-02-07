// link-time-logical-operators.cpp
// Test that logical operators work correctly with link-time constants
// and that extern const values can be overridden properly.

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

void linkTimeLogicalOperatorsTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadProgram(
        device,
        shaderProgram,
        "link-time-logical-operators",
        "computeMain",
        slangReflection,
        R"(
            export static const bool HAS_FEATURE = true;
            export static const bool ENABLE_EXTRA = true;
        )"));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<rhi::IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 1;
    float initialData[] = {0.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> outputBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, outputBuffer.writeRef()));

    // After linking with HAS_FEATURE=true and ENABLE_EXTRA=true:
    // - notValue: Conditional<float, !true> = Conditional<float, false> (empty, contributes 0.0)
    // - andValue: Conditional<float, true && true> = Conditional<float, true> (has value 10.0)
    // - orValue: Conditional<float, true || false> = Conditional<float, true> (has value 20.0)
    // The Conditional values are initialized in the shader code itself

    // Execute the compute shader
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipelineState);

        ShaderCursor rootCursor(rootObject);
        rootCursor.getPath("Output").setBinding(Binding(outputBuffer));

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    // Expected result when HAS_FEATURE=true, ENABLE_EXTRA=true:
    // Test 1: !true = false, notValue is empty, adds 0.0
    // Test 2: true && true = true, andValue has value, adds 10.0
    // Test 3: true || false = true, orValue has value, adds 20.0
    // Total: 0.0 + 10.0 + 20.0 = 30.0
    compareComputeResult(device, outputBuffer, std::array{30.0f});
}

SLANG_UNIT_TEST(linkTimeLogicalOperatorsD3D12)
{
    runTestImpl(linkTimeLogicalOperatorsTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(linkTimeLogicalOperatorsVulkan)
{
    runTestImpl(linkTimeLogicalOperatorsTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
