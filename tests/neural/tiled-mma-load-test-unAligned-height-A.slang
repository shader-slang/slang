// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import slang.neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly when the height of the matrix
// is not aligned with the tile height. The result of the tile will always be aligned with 16 in columns
// with 0 paddings.
//
// ============================================================================
// MATRIX A - Loaded from Global Memory
// ============================================================================
// We construct a 32x32 matrix in row-major order:
//
//   Column:  0    1    2    3   ...  15   16   17   ...   31
//   Row 0:   0    1    2    3   ...  15  16   17   ...    31
//   Row 1:  32   33   34   35   ...  47  48   49   ...   63
//   ...
//   Row 7:  224  225  226  227  ...  239  240  241   ...   255
//   Row 8:  256  257  258  259  ...  269  270  271   ...   287
//   ...
//   Row 14: 448  449  450  451  ...  459  460  461   ...   479
//   Row 15: 480  481  482  483  ...  491  492  493   ...   511
//   ...
//   Row 31: 992  993  994  995  ...  1007 1008 1009  ...   1023
//

// We will test different heights of the matrix, all the heights are smaller than 16.
// For example, if we set the height to 3, the matrix will be:
//
//   0    1    2 ... 15
//  32   33   34 ... 47
//  64   65   66 ... 79
//  [paddings for row 3-15]

// Make the weight matrix as 16x32 matrix in row major order
// TEST_INPUT: set inputBuffer = ubuffer(stride=2, count=1024)
uniform RWStructuredBuffer<half>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=7):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

static const int InputSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (32 * 2) * sizeof(uint4);
}
static const uint OffsetA = 0;

void invalidateSharedMemory(uint tid, uint sharedMemOffset)
{
    if (tid >= SubgroupSize)
        return;

    // Initialize the shared memory with all 1.0h.
    uint activeThreadCount = WaveActiveCountBits(true);
    uint numIters = (32 * 2 + activeThreadCount - 1) / activeThreadCount;
    for (int i = 0; i < numIters; i++)
    {
        uint index = tid * numIters + i;
        if (index >= 32 * 2)
            break;

        neural_unittest.writeSharedMemory<TestShMemSize>(sharedMemOffset + index, uint4(0x3C003C00));
    }
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M>(uint tid, uint size, uint sharedMemOffset, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * 32 + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread
    uint alignedM = ((M + 16 - 1) / 16) * 16;

    if (tid < alignedM)
    {
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = tid * 2 + i;
            uint4 values = neural_unittest.readSharedMemory<TestShMemSize>(sharedMemOffset + indexInTile);
            uint4 element = values;
            if (indexInTile / 2 >= M)
            {
                // Checking paddings are correct for out-of-range elements.
                if (!values.equals(uint4(0, 0, 0, 0)))
                {
                    return false;
                }
                continue;
            }
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half actual[2] = { bit_cast<half>(a), bit_cast<half>(b) };
                half expectedValues[2] = { expected, expected + 1.0h };
                for (int verifyIndex = 0; verifyIndex < 2; verifyIndex++)
                {
                    if (actual[verifyIndex] != expectedValues[verifyIndex])
                    {
                        return false;
                    }
                }
                expected += 2.0h;
            }
        }
    }
    else
    {
        // For out-of-range rows, we just check if the values are same as the initialized values.
        // Because we also need to check if the library accidentally write some values to the out-of-range rows.
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = tid * 2 + i;
            uint4 values = neural_unittest.readSharedMemory<TestShMemSize>(sharedMemOffset + indexInTile);
            if (!values.equals(uint4(0x3C003C00)))
            {
                return false;
            }
        }
    }

    return true;
}

void Test<int M>(uint tid, int resIndex)
{
    invalidateSharedMemory(tid, OffsetA);
    neural_unittest.testLoadShA<InputSize, M, TestShMemSize, false, 1>(OffsetA, 0, inputBuffer);

    bool res = verifiedOutput<M>(tid, M, OffsetA, 0);
    res = WaveActiveAllTrue(res);

    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);

    if (tid >= SubgroupSize)
        return;

    // smaller than 16 cases, it will padded to 16 rows.
    Test<13>(tid, 0);
    Test<9>(tid, 1);
    Test<7>(tid, 2);
    Test<3>(tid, 3);
    Test<1>(tid, 4);

    Test<19>(tid, 5); // bigger than 16 case, it will padded to 32 rows.
    Test<25>(tid, 6);
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
