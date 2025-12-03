// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly when the height of the matrix
// is not aligned with the tile height. The result of the tile will always be aligned with 16 in columns
// with 0 paddings.
//
// We will construct the input vector as follow:
// x = tid * 32 + 0, 1, 2, 3 ... 31

// We will test different heights of the matrix B.
// For example, if we set the height to 19, and read the second tile, the matrix will be:
//
//  16  48    80  ... 1008
//  17  49    81  ... 1009
//  18  50    82  ... 1010
//  [zeros rows for height 3-15]

// Explanation: According the way of how we construct the input vector, the matrix will be:
//
//  0   32    64  ... 992
//  1   33    65  ... 993
//  2   34    66  ... 994
//  ...
//  31  65    97  ... 1023
// in major column.

// So if we set height to 19, the matrix will be:
//
//  0   32    64  ... 992
//  1   33    65  ... 993
//  2   34    66  ... 994
//  ...
//  18  48    78  ... 1008
//  [zeros rows for height 19-31]

// Since we are reading the second tile, it will start from the 16th row, so the matrix will be:
//
//  16  48    80  ... 1008
//  17  49    81  ... 1009
//  18  50    82  ... 1010
//  [zeros rows for height 3-15]


// TEST_INPUT:ubuffer(stride=4, count=5):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int OutputSize = 32;
static const int SubgroupSize = 32;

// Tile A is size of 32 * 32 bytes: TileHeight is at least 16.
groupshared uint4 s_sharedMemoryB[32 * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShB<int K>(uint tid, uint tileIndex)
{
    half inputVector[K];
    for (int i = 0; i < 32; i++)
    {
        inputVector[i] = half(tid * 32 + i);
    }

    SPtr sharedMemoryB = __getAddress(s_sharedMemoryB[0]);
    // This test only has one subgroup, so the subgroup index is always 0.
    MMAHelper<half, K, OutputSize, SubgroupSize>.LoadSharedMemoryFromLocalVector(sharedMemoryB, tileIndex, 0, inputVector);
    GroupMemoryBarrierWithWaveSync();
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

bool verifiedOutput<int K>(uint tid, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * 32 + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread

    for (int i = 0; i < 2; i++)
    {
        uint indexInTile = tid * 2 + i;
        uint4 values = sharedMem[indexInTile];
        uint4 element = values;

        for (int j = 0; j < 4; j++)
        {
            uint yIndex = (i * 8 + j * 2) + tileIndex * 16;
            uint value = element[j];
            uint16_t a = (uint16_t)(value & 0xFFFF);
            uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);
            half actual[2] = { bit_cast<half>(a), bit_cast<half>(b) };
            half expectedValues[2] = { expected, expected + 1.0h };

            for (int verifyIndex = 0; verifyIndex < 2; verifyIndex++)
            {
                // Check out-of-boundary for X direction.
                if (yIndex++ >= K)
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


    return true;
}

void Test<int K>(uint tid, uint tileIndex, int resIndex)
{
    invalidateSharedMemory(tid, __getAddress(s_sharedMemoryB[0]));
    testLoadShB<K>(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryB[0]));

    bool res = verifiedOutput<K>(tid, __getAddress(s_sharedMemoryB[0]), tileIndex);
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

[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid >= SubgroupSize)
        return;

    Test<19>(tid, 0, 0);    // arbitrary case, K = 19, read tile 0
    // BUFFER: 1

    Test<19>(tid, 1, 1);    // arbitrary case, K = 19, read tile 1
    // BUFFER-NEXT: 1

    Test<16>(tid, 0, 2);     // boundary case, K = 16, read tile 0
    // BUFFER-NEXT: 1

    Test<32>(tid, 0, 3);     // boundary case, K = 32, read tile 0
    // BUFFER-NEXT: 1

    Test<32>(tid, 1, 4);     // boundary case, K = 32, read tile 1
    // BUFFER-NEXT: 1
}
