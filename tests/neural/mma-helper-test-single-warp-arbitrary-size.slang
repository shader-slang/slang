// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)

import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif


// TEST_INPUT: set inputBuffer = ubuffer(data=[0], stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(data=[0], stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}

void fillWeightMatrix<int InputSize, int OutputSize>(uint tid)
{
    uint numIter = (OutputSize + BatchSize - 1) / BatchSize;

    for (int i = 0; i < numIter; i++)
    {
        int rowIndex = i * BatchSize + tid;
        if (rowIndex >= OutputSize)
            break;

        int startIndex = rowIndex * InputSize;
        for (int j = 0; j < InputSize; j++)
        {
            int index = startIndex + j;
            inputBuffer[index] = DType(0.5);
        }
    }

    if (tid == 0)
    {
        for (int i = 0; i < 64; i++)
        {
            biasBuffer[i] = DType(i);
        }
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
#if !TEST_BIAS
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, false, 1, false>(
        tid, inputBuffer, none);
#else
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, true, 1, false>(
        tid, inputBuffer, biasBuffer);
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

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    {
        fillWeightMatrix<3, 5>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<3, 5>(tid, 0);
    }

    {
        fillWeightMatrix<11, 18>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<11, 18>(tid, 1);
    }

    {
        fillWeightMatrix<23, 9>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<23, 9>(tid, 2);
    }

    {
        fillWeightMatrix<35, 3>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<35, 3>(tid, 3);
    }

    {
        fillWeightMatrix<17, 64>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
