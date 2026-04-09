// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

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

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}

void identityWeightMatrix<int InputSize>(uint tid, bool clear = false)
{
    if (InputSize <= BatchSize)
    {
        if (tid > InputSize)
            return;
        int index = tid * InputSize + tid;
        if (clear)
            inputBuffer[index] = DType(0);
        else
            inputBuffer[index] = DType(1);
    }
    else
    {
        int numWrites = InputSize / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int index = row * InputSize + row;
            if (clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(1);
        }
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    DType inputData[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if !TEST_BIAS
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, InputSize, TestShMemSize, false, 1, false>(
        inputData, inputBuffer, none);
#else
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, InputSize, TestShMemSize, true, 1, false>(
        inputData, inputBuffer, biasBuffer);
#endif

    bool res = true;
    for (int i = 0; i < OutputSize; i++)
    {
#if !TEST_BIAS
        DType expected = DType(tid + 1);
#else
        DType expected = DType(tid + 1) + DType(i);
#endif
        if (!equals(output[i], expected))
        {
            res = false;
            break;
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
        identityWeightMatrix<4>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<4, 4>(tid, 0);
        identityWeightMatrix<4>(tid, true);
    }

    {
        identityWeightMatrix<8>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<8, 8>(tid, 1);
        identityWeightMatrix<8>(tid, true);
    }

    {
        identityWeightMatrix<16>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<16, 16>(tid, 2);
        identityWeightMatrix<16>(tid, true);
    }

    {
        identityWeightMatrix<32>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<32, 32>(tid, 3);
        identityWeightMatrix<32>(tid, true);
    }

    {
        identityWeightMatrix<64>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<64, 64>(tid, 4);
        identityWeightMatrix<64>(tid, true);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
