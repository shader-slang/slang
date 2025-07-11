#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"
using namespace rhi;

namespace gfx_test
{
Slang::ComPtr<IBuffer> createBuffer(IDevice* device, uint32_t data, ResourceState defaultState)
{
    uint32_t initialData[] = {data, data, data, data};
    const int numberCount = SLANG_COUNT_OF(initialData);
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(initialData);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(uint32_t) * 4;
    bufferDesc.defaultState = defaultState;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    // Set appropriate usage flags based on the default state
    if (defaultState == ResourceState::ShaderResource)
    {
        bufferDesc.usage = BufferUsage::ShaderResource;
    }
    else if (defaultState == ResourceState::UnorderedAccess)
    {
        bufferDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::CopySource;
    }

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));
    return numbersBuffer;
}

struct uint4
{
    uint32_t x, y, z, w;
};

void nestedParameterBlockTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "nested-parameter-block",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<rhi::IComputePipeline> pipeline;
    pipeline = device->createComputePipeline(pipelineDesc);

    ComPtr<IShaderObject> shaderObject;
    SLANG_CHECK(
        SLANG_SUCCEEDED(device->createRootShaderObject(shaderProgram, shaderObject.writeRef())));

    Slang::List<Slang::ComPtr<IBuffer>> srvBuffers;

    for (uint32_t i = 0; i < 6; i++)
    {
        srvBuffers.add(createBuffer(device, i, rhi::ResourceState::ShaderResource));
    }
    Slang::ComPtr<IBuffer> resultBuffer =
        createBuffer(device, 0, rhi::ResourceState::UnorderedAccess);

    Slang::ComPtr<IShaderObject> materialObject;
    SLANG_CHECK(SLANG_SUCCEEDED(device->createShaderObject(
        slangReflection->findTypeByName("MaterialSystem"),
        ShaderObjectContainerType::None,
        materialObject.writeRef())));

    Slang::ComPtr<IShaderObject> sceneObject;
    SLANG_CHECK(SLANG_SUCCEEDED(device->createShaderObject(
        slangReflection->findTypeByName("Scene"),
        ShaderObjectContainerType::None,
        sceneObject.writeRef())));

    ShaderCursor cursor(shaderObject);
    cursor["resultBuffer"].setBinding(Binding(resultBuffer));
    cursor["scene"].setObject(sceneObject);

    Slang::ComPtr<IShaderObject> globalCB;
    SLANG_CHECK(SLANG_SUCCEEDED(device->createShaderObject(
        cursor[0].getTypeLayout()->getType(),
        ShaderObjectContainerType::None,
        globalCB.writeRef())));

    cursor[0].setObject(globalCB);
    auto initialData = uint4{20, 20, 20, 20};
    globalCB->setData(ShaderOffset(), &initialData, sizeof(initialData));

    ShaderCursor sceneCursor(sceneObject);
    sceneCursor["sceneCb"].setData(uint4{100, 100, 100, 100});
    sceneCursor["data"].setBinding(Binding(srvBuffers[1]));
    sceneCursor["material"].setObject(materialObject);

    ShaderCursor materialCursor(materialObject);
    materialCursor["cb"].setData(uint4{1000, 1000, 1000, 1000});
    materialCursor["data"].setBinding(Binding(srvBuffers[2]));

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);

        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();

        encoder->bindPipeline(pipeline, shaderObject);

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, resultBuffer, std::array{1123u, 1123u, 1123u, 1123u});
}

SLANG_UNIT_TEST(nestedParameterBlockTestD3D12)
{
    runTestImpl(nestedParameterBlockTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(nestedParameterBlockTestVulkan)
{
    runTestImpl(nestedParameterBlockTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
