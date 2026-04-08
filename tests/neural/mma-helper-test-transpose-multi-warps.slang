// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

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

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

groupshared bool s_resultMem[BatchSize/32];

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}

void fillWeightMatrix<int InputSize, int OutputSize>(uint tid, bool clear = false)
{
    const int ROW = OutputSize;
    const int COL = InputSize;

    if (ROW <= BatchSize)
    {
        if (tid > ROW)
            return;
        int row = tid;
        int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
        for (int i = 0; i < COL; i++)
        {
            int index = row * COL + i;
            if (clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(scaleFactor * (i + 1));
        }
    }
    else
    {
        int numWrites = ROW / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
            for (int j = 0; j < COL; j++)
            {
                int index = row * COL + j;
                if (clear)
                    inputBuffer[index] = DType(0);
                else
                    inputBuffer[index] = DType(scaleFactor * (j + 1));
            }
        }
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, false, 2, true>(
        tid, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        DType expected = DType(scaleFactor) * DType((i + 1) * (tid + 1));
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }

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
    {
        fillWeightMatrix<4, 4>(tid);
        AllMemoryBarrierWithGroupSync();
        test<4, 4>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrix<8, 8>(tid);
        AllMemoryBarrierWithGroupSync();
        test<8, 8>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrix<16, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrix<32, 32>(tid);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrix<64, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 4);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
