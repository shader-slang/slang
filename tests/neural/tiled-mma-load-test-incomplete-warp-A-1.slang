// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=7
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=7

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=23
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=23

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=31
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=31

import neural;
#pragma warning(disable: 41017)

// This test verifies that if an incomplete warp is launched, the tiled matrix load operations work correctly.
// This test is similar to the tiled-mma-load-test-incomplete-warp-A.slang, but this test is more comprehensive.
// In this test, we will set configure height, width of the matrix A to arbitrary values, and test for different
// thread counts that are smaller than 32.


// ============================================================================
// MATRIX A - Loaded from Global Memory
// ============================================================================
// We construct a 32x32 matrix in row-major order:
//
//   Column:  0    1    2    3   ...  15   16   17   ...   31
//   Row 0:   0    1    2    3   ...  15  16   17   ...    31
//   Row 1:  32   33   34   35   ...  47  48   49   ...    63
//   ...
//   Row 7:  224  225  226  227  ...  239  240  241   ...   255
//   Row 8:  256  257  258  259  ...  269  270  271   ...   287
//   ...
//   Row 14: 448  449  450  451  ...  459  460  461   ...   479
//   Row 15: 480  481  482  483  ...  491  492  493   ...   511
//   ...
//   Row 31: 992  993  994  995  ...  1007 1008 1009  ...   1023


// We will test different number of threads that are smaller than 32.
// For each number of threads, we will configure the height, width of the matrix
// to arbitrary value. The result should still be correct, which is same as
// the case of launching a full warp.
// For example, if we set the number of threads to 7, and the height, width of
// the matrix to 5, 15 respectively,the matrix will be:
//
//   0     1    2   ... 14 0
//   15   16   17   ... 29 0
//   30   31   32   ... 44 0
//   45   46   47   ... 59 0
//   60   61   62   ... 74 0
//  [paddings for row 5-15]


// Make the weight matrix as 16x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int InputSize = 32;
static const int SubgroupSize = 32;

// Tile A is size of 32 * 32 bytes: TileHeight is at least 16.
groupshared uint4 s_sharedMemoryA[32 * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

// Initialize weight matrix from 0-1023
void initWeightMatrix(uint tid)
{
    int activeThreadCount = WaveActiveCountBits(true);
    int numIters = (1024 + activeThreadCount - 1) / activeThreadCount;

    for (int i = 0; i < numIters; i++)
    {
        int index = tid * numIters + i;
        if (index >= 1024)
            break;

        inputBuffer[index] = half(index);
    }
}

void invalidateSharedMemory(uint tid, SPtr shmPtr)
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

        shmPtr[index] = uint4(0x3C003C00);
    }
    GroupMemoryBarrierWithWaveSync();
}

void testLoadShA<int M, int K>(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, K, M, SubgroupSize>.LoadShA<Storage>(sharedMemoryA, tileIndex, storage, 0);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M, int K>(uint tid, uint size, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * K + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread
    uint alignedM = ((M + 16 - 1) / 16) * 16;

    // Verify the output is correct for the aligned rows.
    if (tid < alignedM)
    {
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = tid * 2 + i;
            uint4 values = sharedMem[indexInTile];
            uint4 element = values;
            if (tid >= M) // out-of-range for Y direction, it should be all 0s.
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
                uint xIndex = (i * 8 + j * 2) + tileIndex * 16;
                if (xIndex >= K)
                {
                    if (element[j] != 0)
                    {
                        return false;
                    }
                    continue;
                }
                uint value = element[j];
                uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
                uint16_t b = (uint16_t)(value & 0xFFFF);

                half actual[2] = { bit_cast<half>(a), bit_cast<half>(b) };
                half expectedValues[2] = { expected, expected + 1.0h };
                for (int verifyIndex = 0; verifyIndex < 2; verifyIndex++)
                {
                    if (xIndex++ >= K)
                    {
                        if (actual[verifyIndex] != 0.0h)
                        {
                            return false;
                        }
                        continue;
                    }
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
            uint4 values = sharedMem[indexInTile];
            if (!values.equals(uint4(0x3C003C00)))
            {
                return false;
            }
        }
    }
    return true;
}

void Test<int M, int K>(uint tid, int tileIndex, int resIndex)
{
    if (tid >= THREAD_COUNT)
        return;

    // fill the shared memory with invalid values.
    invalidateSharedMemory(tid, __getAddress(s_sharedMemoryA[0]));

    testLoadShA<M, K>(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput<M, K>(tid, M, __getAddress(s_sharedMemoryA[0]), tileIndex);
    res = WaveActiveAllTrue(res);

    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem)
{
    if (tid > 0)
        return;

    for (int id = 0; id < 32; id++)
    {
        printf("tid: %d\n", id);
        for (int i = 0; i < 2; i++)
        {
            uint4 values = sharedMem[id * 2 + i];
            uint4 element = values;
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
                uint16_t b = (uint16_t)(value & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

[numthreads(THREAD_COUNT, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);

    // Test for different heights of the matrix A.
    Test<5, 15>(tid, 0, 0);  // M == 5, K == 15
    // BUFFER: 1

    Test<16, 3>(tid, 0, 1);  // M == 16, K == 3, read tile 0
    // BUFFER-NEXT: 1

    Test<23, 27>(tid, 0, 2);  // M == 23, K == 27, read tile 1
    // BUFFER-NEXT: 1

    Test<23, 27>(tid, 1, 3);  // M == 23, K == 27, read tile 1
    // BUFFER-NEXT: 1

    Test<32, 32>(tid, 0, 4);  // M == 32, K == 32, read tile 0
    // BUFFER-NEXT: 1
}
