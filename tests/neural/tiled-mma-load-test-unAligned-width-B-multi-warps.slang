// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=40
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=40

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=68
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=68

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled matrix load operations work correctly when the width of the matrix B
// is not aligned with the tile width. Since the width of the tile B is always

// So this test is actually testing the case
// that if an incomplete warp is launched, the tiled matrix load operations work correctly.
//
// Construct the thread local vector as follow:
// x = tid * 32 + 0, 1, 2, 3 ... 31

// We will test different width of the matrix by launching different number of threads that are smaller than 32.
// For example, if we launch 19 threads, the matrix will be:
// Column   0    1     2   ... 18
// Row 0    0    32    64  ... 576
// Row 1    1    33    65  ... 577
// Row 2    2    34    66  ... 578
// ...
// Row 16   16   48    80  ... 592

// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// InputSize is the K dimension of the matrix A, make it unaligned to test the padding logic.
static const int InputSize = 32;                // K
static const int OutputSize = 32;               // M
static const int SubgroupSize = 32;             // N - Note, N dimension is up to the subgroup size.


static const int SUBGROUP_COUNT = (THREAD_COUNT + 31) / 32;
// Tile B is size of 32 * 32 bytes.
static const int TILE_SIZE = 32 * 2;
groupshared uint4 s_sharedMemoryB[SUBGROUP_COUNT * TILE_SIZE];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void initSharedMemoryB(uint tid)
{
    // Initialize the shared memory with all 1.0h.
    // Each subgroup will initialize the shared memory for its own threads.
    uint subgroupIndex = tid / 31;
    uint laneId = WaveGetLaneIndex();
    uint offset = subgroupIndex * TILE_SIZE;
    uint activeThreadCount = WaveActiveCountBits(true);
    uint numIters = (TILE_SIZE + activeThreadCount - 1) / activeThreadCount;
    for (int i = 0; i < numIters; i++)
    {
        uint index = laneId * numIters + i;
        if (index >= TILE_SIZE)
            break;

        s_sharedMemoryB[offset + index] = uint4(0x3C003C00);
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
    MMAHelper<half, InputSize, OutputSize, SubgroupSize>.LoadSharedMemoryFromLocalVector(sharedMemoryB, tileIndex, inputVector);
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
    float expected = subgroupIndex * 1024 + laneId * 32 + tileIndex * 16;
    bool res = true;
    uint index = laneId * 16;  // 16 half per thread
    uint alignedN = ((OutputSize + 16 - 1) / 16) * 16;

    if (laneId < alignedN)
    {
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = laneId * 2 + i;
            uint4 values = sharedMem[indexInTile];
            uint4 element = values;

            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
                uint16_t b = (uint16_t)(value & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);

                if (aa != half(expected))
                {
                    printf("tid: %d, expected: %.1f, actual: %.1f\n", laneId, float(expected), float(aa));
                    return false;
                }

                expected += 1.0h;
                if (bb != half(expected))
                {
                    printf("tid: %d, expected: %.1f, actual: %.1f\n", laneId, float(expected), float(bb));
                    return false;
                }
                expected += 1.0h;
            }
        }
    }
    else
    {
        // For out-of-range rows, we just check if the values are same as the initialized values.
        // Because we also need to check if the library accidentally write some values to the out-of-range rows.
        for (int i = 0; i < 2; i++)
        {
            uint indexInTile = laneId * 2 + i;
            uint4 values = sharedMem[indexInTile];
            if (!values.equals(uint4(0x3C003C00)))
            {
                printf("tid: %d, false 1\n", laneId);
                return false;
            }
        }
    }

    return true;
}

void Test(uint tid, uint tileIndex, int resIndex)
{
    initSharedMemoryB(tid);

    // Each warp will load its own tile.
    testLoadShB(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryB[0]));

    // We will only look at the incomplete warp.
    uint subgroupIndex = tid / 32;
    uint subgroupSize = (THREAD_COUNT + 31) / 32;
    if (subgroupIndex < subgroupSize-1)
        return;

    int laneId = WaveGetLaneIndex();
    uint offset = subgroupIndex * TILE_SIZE;
    bool res = verifiedOutput(laneId, subgroupIndex, __getAddress(s_sharedMemoryB[offset]), tileIndex);
    res = WaveActiveAllTrue(res);

    if (laneId == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem)
{
    GroupMemoryBarrierWithGroupSync();

    if (tid > 0)
        return;

    for (int id = 0; id < SUBGROUP_COUNT * 32; id++)
    {
        printf("row: %d\n", id);
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
