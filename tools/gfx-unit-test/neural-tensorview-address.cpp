#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace rhi;

namespace gfx_test
{

static const int kSlangTorchTensorMaxDim = 5;

struct TensorViewForTest
{
    uint8_t* data;
    uint32_t strides[kSlangTorchTensorMaxDim];
    uint32_t sizes[kSlangTorchTensorMaxDim];
    uint32_t dimensionCount;
};

static TensorViewForTest makeTensorView1D(IBuffer* buffer, uint32_t elementCount, uint32_t elementSize)
{
    TensorViewForTest tv = {};

    NativeHandle handle;
    buffer->getNativeHandle(&handle);
    tv.data = reinterpret_cast<uint8_t*>(handle.value);

    tv.dimensionCount = 1;
    tv.strides[0] = elementSize;
    tv.sizes[0] = elementCount;
    return tv;
}

Slang::ComPtr<IDevice> createCudaDeviceWithExperimentalFeature(UnitTestContext* context)
{
    Slang::ComPtr<IDevice> device;
    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = DeviceType::CUDA;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;

    Slang::List<const char*> searchPaths = getSlangSearchPaths();
    searchPaths.add("../../tools/gfx-unit-test");
    searchPaths.add("tools/gfx-unit-test");
    deviceDesc.slang.searchPaths = searchPaths.getBuffer();
    deviceDesc.slang.searchPathCount = searchPaths.getCount();

    std::vector<slang::CompilerOptionEntry> compilerOptions;
    slang::CompilerOptionEntry experimentalFeature;
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.intValue0 = 1;
    compilerOptions.push_back(experimentalFeature);

    deviceDesc.slang.compilerOptionEntries = compilerOptions.data();
    deviceDesc.slang.compilerOptionEntryCount = compilerOptions.size();

    auto result = getRHI()->createDevice(deviceDesc, device.writeRef());
    if (SLANG_FAILED(result))
        return nullptr;
    return device;
}

static ComPtr<IBuffer> createFloatBuffer(IDevice* device, const float* data, size_t count)
{
    BufferDesc desc = {};
    desc.size = count * sizeof(float);
    desc.elementSize = sizeof(float);
    desc.format = Format::Undefined;
    desc.memoryType = MemoryType::DeviceLocal;
    desc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                 BufferUsage::CopyDestination | BufferUsage::CopySource;
    desc.defaultState = ResourceState::UnorderedAccess;

    ComPtr<IBuffer> buffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(desc, (void*)data, buffer.writeRef()));
    return buffer;
}

void neuralTensorViewAddressTestImpl(IDevice* device, UnitTestContext* context)
{
    // W = [[1,2,3,4],[5,6,7,8]], b = [9, 10]
    float paramsData[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int paramCount = 10;

    auto paramsBuffer = createFloatBuffer(device, paramsData, paramCount);

    // --- Forward test ---
    {
        float outputInit[] = {0, 0};
        auto outputBuffer = createFloatBuffer(device, outputInit, 2);

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadComputeProgram(
            device, shaderProgram, "neural-tensorview-address", "forwardMain", slangReflection));

        ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<IComputePipeline> pipeline;
        GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

        {
            auto queue = device->getQueue(QueueType::Graphics);
            auto commandEncoder = queue->createCommandEncoder();
            auto encoder = commandEncoder->beginComputePass();

            auto rootObject = encoder->bindPipeline(pipeline);
            ShaderCursor rootCursor(rootObject);

            auto paramsTv = makeTensorView1D(paramsBuffer, paramCount, sizeof(float));
            auto outputTv = makeTensorView1D(outputBuffer, 2, sizeof(float));

            rootCursor.getPath("params").setData(&paramsTv, sizeof(paramsTv));
            rootCursor.getPath("output").setData(&outputTv, sizeof(outputTv));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
            queue->submit(commandEncoder->finish());
            queue->waitOnHost();
        }

        // y = W*x + b = [39, 80]
        {
            ComPtr<ISlangBlob> blob;
            device->readBuffer(outputBuffer, 0, 2 * sizeof(float), blob.writeRef());
            auto* r = (const float*)blob->getBufferPointer();
            if (abs(r[0] - 39.0f) > 0.01f || abs(r[1] - 80.0f) > 0.01f)
                fprintf(stderr, "  Forward MISMATCH: y = [%.4f, %.4f] (expected [39.0, 80.0])\n", r[0], r[1]);
        }
        compareComputeResult(device, outputBuffer, std::array{39.0f, 80.0f});
    }

    // --- Backward test ---
    {
        float outputInit[] = {0, 0, 0, 0};
        auto outputBuffer = createFloatBuffer(device, outputInit, 4);

        float gradInit[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto gradParamsBuffer = createFloatBuffer(device, gradInit, paramCount);

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadComputeProgram(
            device, shaderProgram, "neural-tensorview-address", "backwardMain", slangReflection));

        ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<IComputePipeline> pipeline;
        GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

        {
            auto queue = device->getQueue(QueueType::Graphics);
            auto commandEncoder = queue->createCommandEncoder();
            auto encoder = commandEncoder->beginComputePass();

            auto rootObject = encoder->bindPipeline(pipeline);
            ShaderCursor rootCursor(rootObject);

            auto paramsTv = makeTensorView1D(paramsBuffer, paramCount, sizeof(float));
            auto outputTv = makeTensorView1D(outputBuffer, 4, sizeof(float));
            auto gradParamsTv = makeTensorView1D(gradParamsBuffer, paramCount, sizeof(float));

            rootCursor.getPath("params").setData(&paramsTv, sizeof(paramsTv));
            rootCursor.getPath("output").setData(&outputTv, sizeof(outputTv));
            rootCursor.getPath("gradParams").setData(&gradParamsTv, sizeof(gradParamsTv));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
            queue->submit(commandEncoder->finish());
            queue->waitOnHost();
        }

        // dInput = W^T * [1,1] = [1+5, 2+6, 3+7, 4+8] = [6, 8, 10, 12]
        {
            float expected[] = {6.0f, 8.0f, 10.0f, 12.0f};
            ComPtr<ISlangBlob> blob;
            device->readBuffer(outputBuffer, 0, 4 * sizeof(float), blob.writeRef());
            auto* r = (const float*)blob->getBufferPointer();
            for (int i = 0; i < 4; i++)
                if (abs(r[i] - expected[i]) > 0.01f)
                    fprintf(stderr, "  Backward dInput[%d] MISMATCH: %.4f (expected %.1f)\n", i, r[i], expected[i]);
        }
        compareComputeResult(device, outputBuffer, std::array{6.0f, 8.0f, 10.0f, 12.0f});

        // dW = [1,1]^T * [1,2,3,4] = [[1,2,3,4],[1,2,3,4]], dBias = [1, 1]
        {
            float expected[] = {1, 2, 3, 4, 1, 2, 3, 4, 1, 1};
            ComPtr<ISlangBlob> blob;
            device->readBuffer(gradParamsBuffer, 0, paramCount * sizeof(float), blob.writeRef());
            auto* r = (const float*)blob->getBufferPointer();
            for (int i = 0; i < paramCount; i++)
                if (abs(r[i] - expected[i]) > 0.01f)
                    fprintf(stderr, "  Backward gradParams[%d] MISMATCH: %.4f (expected %.1f)\n", i, r[i], expected[i]);
        }
        compareComputeResult(
            device,
            gradParamsBuffer,
            std::array{1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 1.0f});
    }
}

SLANG_UNIT_TEST(neuralTensorViewAddressCUDA)
{
    if (!(unitTestContext->enabledApis & Slang::RenderApiFlag::CUDA))
    {
        SLANG_IGNORE_TEST
    }
    auto device = createCudaDeviceWithExperimentalFeature(unitTestContext);
    if (!device)
    {
        SLANG_IGNORE_TEST
    }
    neuralTensorViewAddressTestImpl(device, unitTestContext);
}

} // namespace gfx_test
