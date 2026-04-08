//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
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

static const int BatchSize = 64;
static const int SubgroupSize = 32;

groupshared bool s_resultMem[BatchSize/32];

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}

void identityWeightMatrix<int InputSize, bool Clear>(uint tid)
{
    if (InputSize <= BatchSize)
    {
        if (tid > InputSize)
            return;
        int index = tid * InputSize + tid;
        if (Clear)
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
            if (Clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(1);
        }
    }
}

bool verifyResult<int OutputSize, T: IArray<DType>>(uint tid, T output)
{
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
    return res;
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
#if !TEST_BIAS
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, false, 2, false>(
        tid, inputBuffer, none);
#else
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, true, 2, false>(
        tid, inputBuffer, biasBuffer);
#endif

    bool res = verifyResult<OutputSize>(tid, output);
    bool finalResult = collectResults<BatchSize>(tid, res);
    if (tid == 0)
    {
        outputBuffer[resIndex] = finalResult ? 1 : 0;
    }
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    {
        identityWeightMatrix<4, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<4, 4>(tid, 0);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<4, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<8, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<8, 8>(tid, 1);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<8, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<16, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<16, 16>(tid, 2);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<32, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<32, 32>(tid, 3);
        AllMemoryBarrierWithGroupSync();

        identityWeightMatrix<32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<64, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<64, 64>(tid, 4);
    }


    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
