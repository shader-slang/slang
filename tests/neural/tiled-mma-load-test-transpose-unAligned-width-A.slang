// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly when the height of the matrix
// is not aligned with the tile height in transpose mode. In transpose mode, the height of the matrix
// is the width of the transposed matrix, so the name of this test.
// The result of the tile will always be aligned with 16 in columns with 0 paddings.
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

// the transposed matrix will be:
// 0    32  64  [paddings for columns 3-15]
// 1    33  65  [paddings for columns 3-15]
// 2    34  66  [paddings for columns 3-15]
// ...
// 15   47  79  [paddings for columns 3-15]

// so the shared memory will be:
// col0: 0 1 2 ... 15
// col1: 1 33 65 ... 79
// col2: 2 34 66 ... 79
// col3: 0 0 0 ... 0
// ...
// col13: 0 0 0 ... 0
// col14: 0 0 0 ... 0
// col15: 0 0 0 ... 0


// Make the weight matrix as 16x32 matrix in row major order
// TEST_INPUT: set inputBuffer = ubuffer(stride=2, count=1024)
uniform RWStructuredBuffer<half>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=9):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

static const int InputSize = 32;
static const int SubgroupSize = 32;

// Tile A is size of InputSize/8 x 16 in uint4.
static const int TileSize = (InputSize / 8) * 16;

// We double the size of the shared memory to also test if there is out-of-bound write issue.
groupshared uint4 s_sharedMemoryA[TileSize * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA<TargetEnum Target, int M>(uint tid, uint tileIndex)
{
    typealias Address = BindlessAddress<half>;

    Address address = Address(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, InputSize, M, SubgroupSize, Target, true>.loadShA<half, Address>(sharedMemoryA, tileIndex, address);
    GroupMemoryBarrierWithWaveSync();
}

void invalidateSharedMemory(uint tid, SPtr shmPtr)
{
    if (tid >= SubgroupSize)
        return;

    // Initialize the shared memory with all 1.0h.
    uint activeThreadCount = WaveActiveCountBits(true);
    uint numIters = (TileSize * 2 + activeThreadCount - 1) / activeThreadCount;
    for (int i = 0; i < numIters; i++)
    {
        uint index = tid * numIters + i;
        if (index >= TileSize * 2)
            break;

        shmPtr[index] = uint4(0x3C003C00);
    }
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M>(uint tid, uint size, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one column of the shared memory.
    // The tile width of A is just 16 for half type.
    const int WMMA_TileWidth = 16;
    const int WMMA_TileHeight = 16;

    const int AlignedK = ((InputSize + WMMA_TileWidth - 1) / WMMA_TileWidth) * WMMA_TileWidth;
    const int TileSizeInElements = AlignedK * WMMA_TileWidth;
    const int ElementCountPerVector = sizeof(uint4) / sizeof(half);
    const int NumVectorsPerColumn = AlignedK / ElementCountPerVector;

    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * AlignedK + tileIndex * TileSizeInElements);
    bool res = true;

    if (tid < WMMA_TileWidth)
    {
        for (int i = 0; i < NumVectorsPerColumn; i++)
        {
            uint indexInTile = tid * NumVectorsPerColumn + i;
            uint4 values = sharedMem[indexInTile];
            uint4 element = values;
            if (indexInTile / NumVectorsPerColumn + tileIndex * WMMA_TileWidth >= M)
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


    {
        // For out-of-range rows, we just check if the values are same as the initialized values.
        // Because we also need to check if the library accidentally write some values to the out-of-range columns.
        int startColumn = (tileIndex + 1) * WMMA_TileWidth;
        int startIndex = startColumn * NumVectorsPerColumn;

        for (int i = 0; i < NumVectorsPerColumn; i++)
        {
            uint indexInTile = (tid * NumVectorsPerColumn + i) + startIndex;
            if (indexInTile >= TileSize * 2)
                break;

            uint4 values = sharedMem[indexInTile];
            if (!values.equals(uint4(0x3C003C00)))
            {
                return false;
            }
        }
    }

    return true;
}

void Test<int M>(uint tid, int tileIndex, int resIndex)
{
    invalidateSharedMemory(tid, __getAddress(s_sharedMemoryA[0]));
    __target_switch
    {
    case cuda:
        testLoadShA<TargetEnum.CUDA, M>(tid, tileIndex);
        break;
    case spirv:
        testLoadShA<TargetEnum.SPIR_V, M>(tid, tileIndex);
        break;
    }
    // serialRead(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput<M>(tid, M, __getAddress(s_sharedMemoryA[0]), tileIndex);
    res = WaveActiveAllTrue(res);

    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

// This function is just used for debugging, not for verification. So keep it here.
// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    // In transpose mode, tile is in column major, and each length is AlignedK length
    for (int id = 0; id < 32; id++)
    {
        printf("col: %d\n", id);
        for (int i = 0; i < 4; i++)
        {
            uint4 values = sharedMem[id * 4 + i];
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
    initWeightMatrix(tid);

    if (tid >= SubgroupSize)
        return;

    // smaller than 16 cases, it will padded to 16 rows.
    Test<13>(tid, 0, 0);
    Test<9>(tid, 0, 1);
    Test<7>(tid, 0, 2);
    Test<3>(tid, 0, 3);
    Test<1>(tid, 0, 4);

    Test<19>(tid, 0, 5); // bigger than 16 case, it will padded to 32 rows.
    Test<19>(tid, 1, 6); // bigger than 16 case, it will padded to 32 rows.
    Test<25>(tid, 0, 7);
    Test<25>(tid, 1, 8);
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
