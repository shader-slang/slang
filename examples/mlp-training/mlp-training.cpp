// In this example, we implement a simple multi-layer perceptron (MLP) training loop on
// Vulkan (through slang-rhi). See also the mlp-training-coopvec example, which
// implements the same MLP training loop using cooperative vector intrinsics for better
// performance.
//
// The simple MLP is trained to approximate a polynomial expression.
// The network contains one hidden layer with 16 neurons. It takes 4 inputs and produces 4
// outputs.

#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "external/slang-rhi/include/slang-rhi.h"
#include "slang-com-ptr.h"
#include "slang.h"

#include <string>

using Slang::ComPtr;

static const ExampleResources resourceBase("mlp-training");

typedef uint16_t NFloat;

static const int kLayerSizes[] = {4, 16, 4};
static const int kLayerCount = sizeof(kLayerSizes) / sizeof(int) - 1;

int getNetworkLayerWeightStride(int i)
{
    return kLayerSizes[i] * sizeof(NFloat);
}

int getNetworkLayerWeightCount(int i)
{
    return kLayerSizes[i] * kLayerSizes[i + 1];
}

int getNetworkLayerBiasCount(int i)
{
    return kLayerSizes[i + 1];
}

struct Kernel
{
    ComPtr<rhi::IShaderProgram> program;
    ComPtr<rhi::IComputePipeline> pipeline;
    operator bool() { return program && pipeline; }
};

struct ClearBufferParams
{
    rhi::DeviceAddress buffer;
    uint32_t count;
};

struct LearnGradParams
{
    rhi::DeviceAddress networkBuffer;
    rhi::DeviceAddress lossBuffer;
    rhi::DeviceAddress inputs;
    uint32_t count;
};

struct AdjustParamsParams
{
    rhi::DeviceAddress adamStates;
    rhi::DeviceAddress params;
    rhi::DeviceAddress gradients;
    uint32_t count;
};

struct ExampleProgram : public TestBase
{
    ComPtr<rhi::IDevice> gDevice;

    ComPtr<slang::ISession> gSlangSession;
    ComPtr<slang::IModule> gSlangModule;
    Kernel gLearnGradProgram;
    Kernel gAdjustParamProgram;

    // Sub-allocated buffer range for each network layer's parameters (weights, biases, gradients).
    //
    struct NetworkParameterAllocation
    {
        size_t weightsOffset;
        size_t weightsSize;
        size_t biasOffset;
        size_t biasSize;
        size_t weightsGradOffset;
        size_t biasGradOffset;
    };

    SlangResult execute(int argc, char* argv[])
    {
        parseOption(argc, argv);

        rhi::DeviceDesc deviceDesc;
        deviceDesc.slang.targetProfile = "spirv_1_6";
        deviceDesc.deviceType = rhi::DeviceType::Vulkan;

        gDevice = rhi::getRHI()->createDevice(deviceDesc);
        if (!gDevice)
            return SLANG_FAIL;

        SLANG_RETURN_ON_FAIL(loadShaderKernels());

        // Create a buffer to hold all network parameters (weights, biases, gradients).
        // This buffer is arranged as following:
        // (segment 1): | weights0 | bias0 | weights1 | bias1 | ... | weightsN | biasN |
        // (segment 2): | weightsGrad0 | biasGrad0 | weightsGrad1 | biasGrad1 | ... |
        //
        // Where the first segment contains all weights and biases for each layer in row-major
        // layout. The second segment contains gradients for weights and biases in row-major layout.

        // Total size of all network parameters.
        size_t paramBufferSize;

        // Offset for the second segment, where gradients for weights and biases in row-major layout
        // start.
        size_t gradientOffset;

        // Sub-allocated weight/Bias offsets for each layer.
        std::vector<NetworkParameterAllocation> layerAllocations;
        allocateNetworkParameterStorage(layerAllocations, paramBufferSize, gradientOffset);

        std::vector<uint16_t> initParams;
        srand(1072);
        for (int i = 0; i < paramBufferSize / sizeof(NFloat); i++)
        {
            if (i < gradientOffset / sizeof(NFloat))
            {
                float v = rand() / (float)RAND_MAX;
                v = v * 2.0f - 1.0f; // Normalize to [-1, 1]
                initParams.push_back(floatToHalf(v));
            }
            else
            {
                // Initialize gradients to zero.
                initParams.push_back(0);
            }
        }
        auto networkParamsBuffer = createBuffer(paramBufferSize, initParams.data());

        static const size_t kAdamStateSize = sizeof(NFloat) * 2 + sizeof(int32_t);
        auto adamStateBuffer = createBuffer(initParams.size() * kAdamStateSize);
        clearBuffer(adamStateBuffer);

        std::vector<uint64_t> networkConstantBufferData;
        auto paramBufferAddr = networkParamsBuffer->getDeviceAddress();
        for (int i = 0; i < kLayerCount; i++)
        {
            networkConstantBufferData.push_back(
                paramBufferAddr + layerAllocations[i].weightsOffset);
            networkConstantBufferData.push_back(
                paramBufferAddr + layerAllocations[i].weightsGradOffset);
            networkConstantBufferData.push_back(paramBufferAddr + layerAllocations[i].biasOffset);
            networkConstantBufferData.push_back(
                paramBufferAddr + layerAllocations[i].biasGradOffset);
        }
        auto networkConstantBuffer = createBuffer(
            networkConstantBufferData.size() * sizeof(uint64_t),
            networkConstantBufferData.data());

        static const int inputCount = 32;
        std::vector<float> inputBufferData;
        for (int i = 0; i < inputCount; i++)
        {
            inputBufferData.push_back(rand() / static_cast<float>(RAND_MAX));
        }
        auto inputBuffer = createBuffer(inputCount * sizeof(float), inputBufferData.data());

        // Create buffer for receiving current loss value.
        auto lossBuffer = createBuffer(sizeof(uint64_t));

        auto queue = gDevice->getQueue(rhi::QueueType::Graphics);

        for (int k = 0; k < 1000; k++)
        {
            clearBuffer(lossBuffer);

            // Compute gradients.
            {
                LearnGradParams entryPointParams = {};
                entryPointParams.inputs = inputBuffer->getDeviceAddress();
                entryPointParams.count = inputCount / 2;
                entryPointParams.lossBuffer = lossBuffer->getDeviceAddress();
                entryPointParams.networkBuffer = networkConstantBuffer->getDeviceAddress();
                dispatchKernel(
                    gLearnGradProgram,
                    entryPointParams,
                    (entryPointParams.count + 255) / 256);
            }
            // Adjust parameters in row-major buffer (adam optimize).
            {
                AdjustParamsParams entryPointParams = {};
                entryPointParams.adamStates = adamStateBuffer->getDeviceAddress();
                entryPointParams.params = networkParamsBuffer->getDeviceAddress();
                entryPointParams.count = (paramBufferSize - gradientOffset) / sizeof(NFloat);
                entryPointParams.gradients =
                    networkParamsBuffer->getDeviceAddress() + gradientOffset;
                dispatchKernel(
                    gAdjustParamProgram,
                    entryPointParams,
                    (entryPointParams.count + 255) / 256);
            }
            if ((k + 1) % 10 == 0)
            {
                queue->waitOnHost();
                ComPtr<ISlangBlob> blob;
                gDevice->readBuffer(lossBuffer, 0, sizeof(float), blob.writeRef());
                printf("Loss after %d iterations: %f\n", k + 1, *(float*)blob->getBufferPointer());
            }
        }
        return SLANG_OK;
    }

    // Allocate storage for network parameters, including weights, biases, and gradients.
    void allocateNetworkParameterStorage(
        std::vector<NetworkParameterAllocation>& paramStorage,
        size_t& outParamBufferSize,
        size_t& outGradientOffset)
    {
        outParamBufferSize = 0;

        auto allocRowMajorStorage = [&](size_t size)
        {
            size = (size + 63) / 64 * 64;
            size_t offset = outParamBufferSize;
            outParamBufferSize += size;
            return offset;
        };

        for (int i = 0; i < kLayerCount; i++)
        {
            size_t biasSize = getNetworkLayerBiasCount(i) * sizeof(NFloat);
            NetworkParameterAllocation layer = {};
            layer.weightsSize = getNetworkLayerWeightCount(i) * sizeof(NFloat);
            layer.weightsOffset = allocRowMajorStorage(layer.weightsSize);
            layer.biasSize = biasSize;
            layer.biasOffset = allocRowMajorStorage(biasSize);
            paramStorage.push_back(layer);
        }

        // Alloc storage for gradients.
        outGradientOffset = outParamBufferSize;
        for (int i = 0; i < kLayerCount; i++)
        {
            paramStorage[i].weightsGradOffset = allocRowMajorStorage(paramStorage[i].weightsSize);
            paramStorage[i].biasGradOffset = allocRowMajorStorage(paramStorage[i].biasSize);
        }
    }

    template<typename Args>
    void dispatchKernel(Kernel& kernel, Args& args, size_t numWorkGroups)
    {
        auto queue = gDevice->getQueue(rhi::QueueType::Graphics);
        ComPtr<rhi::ICommandEncoder> encoder;
        queue->createCommandEncoder(encoder.writeRef());
        {
            auto computeEncoder = encoder->beginComputePass();
            auto rootShaderObject = computeEncoder->bindPipeline(kernel.pipeline.get());
            rootShaderObject->getEntryPoint(0)->setData(rhi::ShaderOffset(), &args, sizeof(args));
            computeEncoder->dispatchCompute(numWorkGroups, 1, 1);
            computeEncoder->end();
        }
        ComPtr<rhi::ICommandBuffer> commandBuffer;
        encoder->finish(commandBuffer.writeRef());
        queue->submit(commandBuffer);
    }

    // Create a buffer with the specified size and optional initial data.
    ComPtr<rhi::IBuffer> createBuffer(size_t size, void* initData = nullptr)
    {
        rhi::BufferDesc bufferDesc = {};
        bufferDesc.size = size;
        bufferDesc.defaultState = rhi::ResourceState::UnorderedAccess;
        bufferDesc.usage = rhi::BufferUsage::CopySource | rhi::BufferUsage::CopyDestination |
                           rhi::BufferUsage::UnorderedAccess;
        bufferDesc.memoryType = rhi::MemoryType::DeviceLocal;
        return gDevice->createBuffer(bufferDesc, initData);
    }

    void clearBuffer(rhi::IBuffer* buffer)
    {
        auto queue = gDevice->getQueue(rhi::QueueType::Graphics);
        auto encoder = queue->createCommandEncoder();
        encoder->clearBuffer(buffer);
        auto cmdBuffer = encoder->finish();
        queue->submit(cmdBuffer);
    }

    Kernel loadComputeProgram(slang::IModule* slangModule, char const* entryPointName)
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        slangModule->findEntryPointByName(entryPointName, entryPoint.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        entryPoint->link(linkedProgram.writeRef());

        if (isTestMode())
        {
            printEntrypointHashes(1, 1, linkedProgram);
        }

        Kernel result;

        rhi::ComputePipelineDesc desc;
        auto program = gDevice->createShaderProgram(linkedProgram);
        desc.program = program.get();
        result.program = program;
        result.pipeline = gDevice->createComputePipeline(desc);
        return result;
    }

    inline unsigned short floatToHalf(float val)
    {
        uint32_t x = 0;
        memcpy(&x, &val, sizeof(float));

        unsigned short bits = (x >> 16) & 0x8000;
        unsigned short m = (x >> 12) & 0x07ff;
        unsigned int e = (x >> 23) & 0xff;
        if (e < 103)
            return bits;
        if (e > 142)
        {
            bits |= 0x7c00u;
            bits |= e == 255 && (x & 0x007fffffu);
            return bits;
        }
        if (e < 113)
        {
            m |= 0x0800u;
            bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
            return bits;
        }
        bits |= ((e - 112) << 10) | (m >> 1);
        bits += m & 1;
        return bits;
    }

    ComPtr<slang::ISession> createSlangSession(rhi::IDevice* device)
    {
        ComPtr<slang::ISession> slangSession = device->getSlangSession();
        return slangSession;
    }

    ComPtr<slang::IModule> compileShaderModuleFromFile(
        slang::ISession* slangSession,
        char const* filePath)
    {
        ComPtr<slang::IModule> slangModule;
        ComPtr<slang::IBlob> diagnosticBlob;
        Slang::String path = resourceBase.resolveResource(filePath);
        slangModule = slangSession->loadModule(path.getBuffer(), diagnosticBlob.writeRef());
        diagnoseIfNeeded(diagnosticBlob);

        return slangModule;
    }

    SlangResult loadShaderKernels()
    {
        Slang::String path = resourceBase.resolveResource("kernels.slang");

        gSlangSession = createSlangSession(gDevice);
        gSlangModule = compileShaderModuleFromFile(gSlangSession, path.getBuffer());
        if (!gSlangModule)
            return SLANG_FAIL;

        gLearnGradProgram = loadComputeProgram(gSlangModule, "learnGradient");
        if (!gLearnGradProgram)
            return SLANG_FAIL;

        gAdjustParamProgram = loadComputeProgram(gSlangModule, "adjustParameters");
        if (!gAdjustParamProgram)
            return SLANG_FAIL;

        return SLANG_OK;
    }
};

int exampleMain(int argc, char** argv)
{
    ExampleProgram app;
    if (SLANG_FAILED(app.execute(argc, argv)))
    {
        return -1;
    }
    return 0;
}
