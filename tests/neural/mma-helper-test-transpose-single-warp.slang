// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

// This test verifies that the tiled MMA load operations work correctly.
//
// ============================================================================
// MATRIX A - Loaded from Global Memory
// ============================================================================
// We construct a 32x32 matrix in row-major order:
//
//   Column:  0    1    2    3   ...  15   16   17   ...   31
//   Row 0:   0    1    2    3   ...  15   16   17   ...   31
//   Row 1:  32   33   34   35   ...  47   48   49   ...   63
//   ...
//   Row 30: 960  961  962  963  ...  975  976  977  ...  991
//   Row 31: 992  993  994  995  ... 1007 1008 1009  ... 1023
//


// Make the weight matrix a 64x64 matrix in row major order

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

// Allocate large enough shared memory in this test.
public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}

// Initialize the weight with the pattern:
// each column is filled with -1 0 1 * column_index in a cycle.
void fillWeightMatrix<int InputSize, int OutputSize>(uint tid, bool clear = false)
{
    if (OutputSize <= BatchSize)
    {
        if (tid > OutputSize)
            return;
        int row = tid;
        int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
        for (int i = 0; i < InputSize; i++)
        {
            int index = row * InputSize + i;
            if (clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(scaleFactor * (i + 1));
        }
    }
    else
    {
        int numWrites = InputSize / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
            for (int j = 0; j < InputSize; j++)
            {
                int index = row * InputSize + j;
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
    let output = neural_unittest.testMatVecMul<DType, InputSize, OutputSize, TestShMemSize, false, 1, true>(tid, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        DType expected = DType(scaleFactor * (i + 1.0f) * (tid + 1.0f));
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
    {
        fillWeightMatrix<4, 4>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<4, 4>(tid, 0);
        fillWeightMatrix<4, 4>(tid, true);
    }

    {
        fillWeightMatrix<8, 8>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<8, 8>(tid, 1);
        fillWeightMatrix<8, 8>(tid, true);
    }

    {
        fillWeightMatrix<15, 15>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<15, 15>(tid, 2);
        fillWeightMatrix<15, 15>(tid, true);
    }

    {
        fillWeightMatrix<32, 32>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<32, 32>(tid, 3);
        fillWeightMatrix<32, 32>(tid, true);
    }

    {
        fillWeightMatrix<64, 64>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<64, 64>(tid, 4);
        fillWeightMatrix<64, 64>(tid, true);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
