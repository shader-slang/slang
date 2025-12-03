// // TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=64
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=64

import neural;
#pragma warning(disable: 41017)

// Similar to the test tiled-mma-load-test-unAligned-width-B.slang, but this test is testing for multi-warps case.

// If we launch N warps, where N > 1, it's sure that the first N-1 warps are full of 32 threads. But the N-th warp
// may be incomplete, so we will check the last warp to make sure that the tiled matrix load operations work correctly.

// In addition, each subgroup will load its own tile to the shared memory cross the whole workgroup. So each subgroup
// will own its own contiguous chunk of the shared memory offset by TILE_SIZE. So we also need to check the loading is
// correct.

//
// Construct the thread local vector as follow:
// x = tid * 32 + 0, 1, 2, 3 ... 31

// So the whole matrix B (Assume THREAD_COUNT = 40) will be:
// Column   0    1     2   ... 31   32  ... 39
//          0    32    64  ... 992 1024 ... 1248
//          1    33    65  ... 993 1025 ... 1249
//          2    34    66  ... 994 1026 ... 1250
//          ...
//          31   63    95  ... 995 1027 ... 1251
//          32   64    96  ... 996 1028 ... 1252
//
// There are totally 2 warps launched, the first warp will load the first 32 columns, and the second warp will load the remaining 8 columns.
// The first warp will load the following columns:
// Column   0    1     2   ... 31
//          0    32    64  ... 992
//          1    33    65  ... 993
//          2    34    66  ... 994
//          ...
//          31   63    95  ... 995
//
// The second warp will also load the remaining 32 columns.
// Column   32    32    ... 63
//          1024 1056   ... 2016
//          1025 1057   ... 2017
//          1026 1058   ... 2018
//          ...
//          1055 1087   ... 2047

// We only need to check the last warp. As long as all the values are correct, we can assume that the first N-1 warps are correct
// as well.


// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=2):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// InputSize is the K dimension of the matrix A, make it unaligned to test the padding logic.
static const int InputSize = 32;                // K
static const int OutputSize = 32;               // M
static const int SubgroupSize = 32;             // N - Note, N dimension is up to the subgroup size.

static const int SUBGROUP_COUNT = THREAD_COUNT / 32;

// Tile B is size of 16 * 32 bytes.
static const int TILE_SIZE = 16 * 2;
static const int N_TILES_PER_SUBGROUP = 2;
groupshared uint4 s_sharedMemoryB[SUBGROUP_COUNT * N_TILES_PER_SUBGROUP * TILE_SIZE];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void invalidateSharedMemory(uint tid, SPtr shmPtr)
{
    // Initialize the shared memory with all 1.0h.
    // Each subgroup will initialize the shared memory for its own threads.
    uint subgroupIndex = tid / 32;
    uint laneId = WaveGetLaneIndex();
    uint offset = subgroupIndex * (TILE_SIZE * N_TILES_PER_SUBGROUP);
    uint activeThreadCount = WaveActiveCountBits(true);
    uint numIters = ((TILE_SIZE * N_TILES_PER_SUBGROUP) + activeThreadCount - 1) / activeThreadCount;
    uint offsetPerThread = activeThreadCount;

    for (int i = 0; i < numIters; i++)
    {
        uint index = i * offsetPerThread + laneId;
        if (index >= TILE_SIZE * N_TILES_PER_SUBGROUP)
            break;

        shmPtr[offset + index] = uint4(0x3C003C00);
    }

    GroupMemoryBarrierWithWaveSync();
}

void testLoadShB(uint tid, uint tileIndex)
{
    half inputVector[InputSize];
    float value = tid * 32;
    for (int i = 0; i < 32; i++)
    {
        inputVector[i] = half(value + i);
    }

    SPtr sharedMemoryB = __getAddress(s_sharedMemoryB[0]);

    uint subgroupIndex = tid / 32;
    MMAHelper<half, InputSize, OutputSize, SubgroupSize>.LoadSharedMemoryFromLocalVector(sharedMemoryB, tileIndex, subgroupIndex, inputVector);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput(uint laneId, uint subgroupIndex, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    // !!! IMPORTANT: the expected value is a float, not a half, because when the number is bigger than 2048,
    // the half type + 1.0h will always 2048. But by using float, half(2048 + 1.0f) will be 2048.0h, but
    // float(2048 + 2.0f) will be 2050.0f. This is how to construct the input vector in testLoadShB function.
    // So using the same method to verify the output.
    float expected = (subgroupIndex * 1024) + laneId * 32 + tileIndex * 16;
    bool res = true;
    uint index = laneId * 16;  // 16 half per thread
    uint alignedN = ((OutputSize + 16 - 1) / 16) * 16;

    // Because of our setting, the laneId will never be larger than alignedN
    {
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = laneId * 2 + i;
            uint4 values = sharedMem[indexInTile];
            uint4 element = values;

            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);
                half actual[2] = { bit_cast<half>(a), bit_cast<half>(b) };
                half expectedValues[2] = { half(expected), half(expected + 1.0f) };
                for (int verifyIndex = 0; verifyIndex < 2; verifyIndex++)
                {
                    if (actual[verifyIndex] != expectedValues[verifyIndex])
                    {
                        // printf("subgroup %d, actual: %.1f, expected: %.1f\n", subgroupIndex, float(actual[verifyIndex]), float(expectedValues[verifyIndex]));
                        return false;
                    }
                }
                expected += 2.0h;
            }
        }
    }

    // Next we need to check if paddings are correct.
    {
        uint activeThreadCount = WaveActiveCountBits(true);
        uint startColumn = activeThreadCount;
        uint remainningColumns = alignedN - startColumn;
        // Let each thread check one column.
        uint columnsPerThread = (remainningColumns + activeThreadCount - 1) / activeThreadCount;
        for (uint i = 0; i < columnsPerThread; i++)
        {
            uint columnIndex = startColumn + laneId;
            if (columnIndex >= alignedN)
                break;

            uint index = columnIndex * 2;
            if (!sharedMem[index].equals(uint4(0)) || !sharedMem[index+1].equals(uint4(0)))
            {
                return false;
            }
        }
    }

    // Next we need to check if there is any accidental load out of the range.
    if (alignedN < 32)
    {
        uint activeThreadCount = WaveActiveCountBits(true);
        uint startColumn = alignedN;
        uint remainningColumns = 32 - startColumn;
        // Let each thread check one column.
        uint columnsPerThread = (remainningColumns + activeThreadCount - 1) / activeThreadCount;
        for (uint i = 0; i < columnsPerThread; i++)
        {
            uint columnIndex = startColumn + laneId;
            if (columnIndex >= 32)
                break;
            uint index = columnIndex * 2;
            if (!sharedMem[index].equals(uint4(0x3C003C00)) || !sharedMem[index+1].equals(uint4(0x3C003C00)))
            {
                return false;
            }
        }
    }

    return true;
}

groupshared bool s_verifiedOutput[THREAD_COUNT/32];
void Test(uint tid, uint tileIndex, int resIndex)
{
    // Waiting for all threads to finish the previous test as this is multi-warps test.
    // So need to sync whole workgroup.
    GroupMemoryBarrierWithGroupSync();
    invalidateSharedMemory(tid, __getAddress(s_sharedMemoryB[0]));

    // Each warp will load its own tile.
    testLoadShB(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryB[0]));

    uint subgroupIndex = tid / 32;
    int laneId = WaveGetLaneIndex();
    uint offset = subgroupIndex * (TILE_SIZE * N_TILES_PER_SUBGROUP);
    bool res = verifiedOutput(laneId, subgroupIndex, __getAddress(s_sharedMemoryB[offset]), tileIndex);
    res = WaveActiveAllTrue(res);

    // Write verified result to the shared memory.
    if (laneId == 0)
        s_verifiedOutput[subgroupIndex] = res;

    // Wait for all threads to finish writing.
    GroupMemoryBarrierWithGroupSync();

    // Read verified result from the shared memory.
    if (tid == 0)
    {
        for (int i = 0; i < THREAD_COUNT/32; i++)
        {
            if (!s_verifiedOutput[i])
            {
                outputBuffer[resIndex] = 0;
                return;
            }
        }
        outputBuffer[resIndex] = 1;
    }
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem)
{
    GroupMemoryBarrierWithGroupSync();

    if (tid > 0)
        return;

    uint columnsPerTile = 16;
    for (int id = 0; id < SUBGROUP_COUNT * N_TILES_PER_SUBGROUP * columnsPerTile; id++)
    {
        printf("col: %d\n", id);
        for (int i = 0; i < 2; i++)
        {
            uint4 values = sharedMem[id * 2 + i];
            uint4 element = values;
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

RWStructuredBuffer<half> outputBuffer12;

[numthreads(THREAD_COUNT, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    Test(tid, 0, 0);    // arbitrary case, N = THREAD_COUNT, read tile 0
    // BUFFER: 1

    Test(tid, 1, 1);    // arbitrary case, N = THREAD_COUNT, read tile 1
    // BUFFER-NEXT: 1
}
