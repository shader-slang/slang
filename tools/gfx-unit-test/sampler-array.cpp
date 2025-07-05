// Duplicated: This test is identical to slang-rhi\tests\test-sampler-array.cpp

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
static ComPtr<IBuffer> createBuffer(IDevice* device, uint32_t content)
{
    ComPtr<IBuffer> buffer;
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    GFX_CHECK_CALL_ABORT(device->createBuffer(bufferDesc, (void*)&content, buffer.writeRef()));

    return buffer;
}
void samplerArrayTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(
        loadComputeProgram(device, shaderProgram, "sampler-array", "computeMain", slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

    Slang::List<ComPtr<ISampler>> samplers;
    ComPtr<ITexture> texture;
    ComPtr<IBuffer> buffer = createBuffer(device, 0);

    {
        TextureDesc textureDesc = {};
        textureDesc.type = TextureType::Texture2D;
        textureDesc.format = Format::RGBA8Unorm;
        textureDesc.size.width = 2;
        textureDesc.size.height = 2;
        textureDesc.size.depth = 1;
        textureDesc.mipCount = 2;
        textureDesc.memoryType = MemoryType::DeviceLocal;
        textureDesc.usage = TextureUsage::ShaderResource | TextureUsage::CopyDestination;
        textureDesc.defaultState = ResourceState::ShaderResource;
        uint32_t data[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
        SubresourceData subResourceData[2] = {{data, 8, 16}, {data, 8, 16}};
        GFX_CHECK_CALL_ABORT(
            device->createTexture(textureDesc, subResourceData, texture.writeRef()));
    }

    for (uint32_t i = 0; i < 32; i++)
    {
        SamplerDesc desc = {};
        ComPtr<ISampler> sampler;
        GFX_CHECK_CALL_ABORT(device->createSampler(desc, sampler.writeRef()));
        samplers.add(sampler);
    }

    ComPtr<IShaderObject> rootObject;
    device->createRootShaderObject(shaderProgram, rootObject.writeRef());

    ComPtr<IShaderObject> g;
    device->createShaderObject(
        slangReflection->findTypeByName("S0"),
        ShaderObjectContainerType::None,
        g.writeRef());

    ComPtr<IShaderObject> s1;
    device->createShaderObject(
        slangReflection->findTypeByName("S1"),
        ShaderObjectContainerType::None,
        s1.writeRef());

    {
        auto cursor = ShaderCursor(s1);
        for (uint32_t i = 0; i < 32; i++)
        {
            cursor["samplers"][i].setBinding(Binding(samplers[i]));
            cursor["tex"][i].setBinding(Binding(texture));
        }
        cursor["data"].setData(1.0f);
    }
    s1->finalize();

    {
        auto cursor = ShaderCursor(g);
        cursor["s"].setObject(s1);
        cursor["data"].setData(2.0f);
    }

    {
        auto cursor = ShaderCursor(rootObject);
        cursor["g"].setObject(g);
        cursor["buffer"].setBinding(Binding(buffer));
    }
    g->finalize();

    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto passEncoder = commandEncoder->beginComputePass();
        auto rootObject = passEncoder->bindPipeline(pipeline);
        auto cursor = ShaderCursor(rootObject);
        cursor["g"].setObject(g);
        cursor["buffer"].setBinding(buffer);
        passEncoder->dispatchCompute(1, 1, 1);
        passEncoder->end();

        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, buffer, std::array{4.0f});
}

SLANG_UNIT_TEST(samplerArrayVulkan)
{
    runTestImpl(samplerArrayTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
