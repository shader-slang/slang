// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

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

// TEST_INPUT: set weightBuffer = ubuffer(data=[0], stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle weightBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(data=[0], stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasBuffer;

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
    fillForwardWeightTiled<DType, InputSize, OutputSize, BatchSize>(tid, weightBuffer, DType(0.5));
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    DType inputData[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 1, false>(
        inputData, weightBuffer, Optional<RWStructuredBuffer<DType>.Handle>(biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, false>(
        inputData, weightBuffer, none);
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
                if (tid == 0)
                    printf("FAIL resIndex=%d i=%d expected=%f got=%f\n", resIndex, i, float(expected), float(output[i]));
                res = false;
                break;
            }
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    // 3x5 (InputSize=3, OutputSize=5, padded to 16x16)
    {
        fillWeightMatrixTiled<3, 5>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
    }

    // 11x18 (padded to 16x32)
    {
        fillWeightMatrixTiled<11, 18>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
    }

    // 23x9 (padded to 32x16)
    {
        fillWeightMatrixTiled<23, 9>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
    }

    // 35x3 (padded to 48x16)
    {
        fillWeightMatrixTiled<35, 3>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
    }

    // 17x64 (padded to 32x64)
    {
        fillWeightMatrixTiled<17, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
