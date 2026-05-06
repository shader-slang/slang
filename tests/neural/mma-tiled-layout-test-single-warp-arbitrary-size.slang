// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)

import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

struct TestBuffers
{
    RWStructuredBuffer<DType>.Handle weightBuffer;
    RWStructuredBuffer<DType>.Handle biasBuffer;
}

// TEST_INPUT: set buffers.weightBuffer = ubuffer(data=[0], stride=4, count=4096)
// TEST_INPUT: set buffers.biasBuffer = ubuffer(data=[0], stride=4, count=64)
ParameterBlock<TestBuffers> buffers;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

void fillWeightMatrixTiled<int InputSize, int OutputSize>(uint tid)
{
    fillForwardWeightTiled<DType, InputSize, OutputSize, BatchSize>(tid, buffers.weightBuffer, DType(0.5));
}

bool test<int InputSize, int OutputSize>(uint tid)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    typealias InputVec = WaveTangledVector<DType, TestShMemSize, PaddedInputSize, SubgroupSize, 1>;
    InputVec inputData = InputVec(DType(0));
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 1, false>(
        inputData, buffers.weightBuffer, Optional<RWStructuredBuffer<DType>.Handle>(buffers.biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, false>(
        inputData, buffers.weightBuffer, none);
#endif

    bool res = true;
    if (tid < OutputSize)
    {
        for (int i = 0; i < OutputSize; i++)
        {
#if !TEST_BIAS
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5);
#else
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5) + DType(i);
#endif
            if (!equals(output[i], expected))
            {
                res = false;
                break;
            }
        }
    }

    return WaveActiveAllTrue(res);
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex, uint3 gid : SV_GroupID)
{
    if (gid.x != 0)
        return;

    initBias<DType, 64, BatchSize>(tid, buffers.biasBuffer);

    // 3x5 (InputSize=3, OutputSize=5, padded to 16x16)
    {
        fillWeightMatrixTiled<3, 5>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<3, 5>(tid);
        if (tid == 0)
            outputBuffer[0] = result ? 1 : 0;
    }

    // 11x18 (padded to 16x32)
    {
        fillWeightMatrixTiled<11, 18>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<11, 18>(tid);
        if (tid == 0)
            outputBuffer[1] = result ? 1 : 0;
    }

    // 23x9 (padded to 32x16)
    {
        fillWeightMatrixTiled<23, 9>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<23, 9>(tid);
        if (tid == 0)
            outputBuffer[2] = result ? 1 : 0;
    }

    // 35x3 (padded to 48x16)
    {
        fillWeightMatrixTiled<35, 3>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<35, 3>(tid);
        if (tid == 0)
            outputBuffer[3] = result ? 1 : 0;
    }

    // 17x64 (padded to 32x64)
    {
        fillWeightMatrixTiled<17, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<17, 64>(tid);
        if (tid == 0)
            outputBuffer[4] = result ? 1 : 0;
    }
    // BUFFER: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
}
