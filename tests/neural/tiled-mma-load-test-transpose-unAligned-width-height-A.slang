// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly when the width, height of the matrix
// is not aligned with the tile width, height. The result of the tile will always be aligned with 16 each row
// and column with 0 paddings.

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
//   Row 7:  224  225  226  227  ...  239  240  241   ...   255
//   Row 8:  256  257  258  259  ...  269  270  271   ...   287
//   ...
//   Row 14: 448  449  450  451  ...  459  460  461   ...   479
//   Row 15: 480  481  482  483  ...  491  492  493   ...   511
//   ...
//   Row 31: 992  993  994  995  ...  1007 1008 1009  ...   1023
//

// We will test different widths, heights of the matrix, for example,
// if the width == 19, and height == 3, and if we read the 2nd tile, the result will be:
//
//  16   17   18  ...  0 [13 zeros padding]
//  35   36   37  ...  0 [13 zeros padding]
//  54   55   56  ...  0 [13 zeros padding]
// ...
// [13 zeros padding rows]

// Explanation:
// Think about the input matrix is a 1-D buffer from 0-1023, since the width of the matrix is 19 now,
// the stride of the matrix is 19. And height is 3, so the matrix will be:
// So the matrix will be:
// 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
// 19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37
// 38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56

// Because we are reading the 2nd tile, so it will start from the 17th column (index 16), so there
// will be only 3 elements left in each row, the remaining 13 elements will be padded with 0:
//
//  16   17   18  ...  0 [13 zeros padding]
//  35   36   37  ...  0 [13 zeros padding]
//  54   55   56  ...  0 [13 zeros padding]
// ...
// [13 zeros padding rows]


// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=7):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

void invalidateSharedMemoryA(uint tid)
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

        s_sharedMemoryA[index] = uint4(0x3C003C00);
    }
    GroupMemoryBarrierWithWaveSync();
}

static const int SubgroupSize = 32;

static const int TileSize = 4 * 16;
groupshared uint4 s_sharedMemoryA[TileSize * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA<TargetEnum Target, int M, int K>(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, K, M, SubgroupSize, Target, true>.loadShA<half, Storage>(sharedMemoryA, tileIndex, storage, 0);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M, int K>(uint tid, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one column of the shared memory.
    // The tile width of A is just 16 for half type.
    const int WMMA_TileWidth = 16;

    const int AlignedK = ((K + WMMA_TileWidth - 1) / WMMA_TileWidth) * WMMA_TileWidth;
    const int TileSizeInElements = AlignedK * WMMA_TileWidth;
    const int ElementCountPerVector = sizeof(uint4) / sizeof(half);
    const int NumVectorsPerColumn = AlignedK / ElementCountPerVector;

    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    half expected = half(tid * K + (tileIndex * K * WMMA_TileWidth));
    bool res = true;

    if (tid < WMMA_TileWidth)
    {
        for (int i = 0; i < NumVectorsPerColumn; i++)
        {
            uint indexInTile = tid * NumVectorsPerColumn + i;
            uint4 element = sharedMem[indexInTile];
            if (indexInTile / NumVectorsPerColumn + tileIndex * WMMA_TileWidth >= M)
            {
                // Checking paddings are correct for out-of-range elements.
                if (!element.equals(uint4(0, 0, 0, 0)))
                {
                    return false;
                }
                continue;
            }

            uint yIndex = ElementCountPerVector * i;

            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half actual[2] = { bit_cast<half>(a), bit_cast<half>(b) };
                half expectedValues[2] = { expected, expected + 1.0h };
                for (int verifyIndex = 0; verifyIndex < 2; verifyIndex++)
                {
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

void Test<int M, int K>(uint tid, uint tileIndex, int resIndex)
{
    if (tid >= SubgroupSize)
        return;

    // fill the shared memory with invalid values.
    invalidateSharedMemoryA(tid);

    __target_switch
    {
    case cuda:
        testLoadShA<TargetEnum.CUDA, M, K>(tid, tileIndex);
        break;
    case spirv:
        testLoadShA<TargetEnum.SPIR_V, M, K>(tid, tileIndex);
        break;
    }
    // serialRead<K>(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput<M, K>(tid, __getAddress(s_sharedMemoryA[0]), tileIndex);
    res = WaveActiveAllTrue(res);

    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<int K>(uint tid, SPtr sharedMem)
{
    if (tid > 0)
        return;

    uint AlignedK = ((K + 16 - 1) / 16) * 16;
    uint NumVectorsPerColumn = AlignedK / 8;

    for (int col = 0; col < 16; col++)
    {
        printf("col: %d\n", col);
        for (int i = 0; i < NumVectorsPerColumn; i++)
        {
            uint4 values = sharedMem[col * NumVectorsPerColumn + i];
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

    Test<3, 19>(tid, 0, 0);      // M == 3, K == 19, read tile 1
    // BUFFER: 1

    Test<19, 5>(tid, 0, 1);      // M == 19, K == 5, read tile 0
    // BUFFER-NEXT: 1

    Test<19, 5>(tid, 1, 2);      // M == 19, K == 5, read tile 1
    // BUFFER-NEXT: 1

    Test<5, 32>(tid, 0, 3);      // M == 5, K == 32, read tile 0
    // BUFFER-NEXT: 1

    Test<14, 13>(tid, 0, 4);     // M == 14, K == 13, read tile 0
    // BUFFER-NEXT: 1

    Test<25, 16>(tid, 0, 5);     // M == 25, K == 16, read tile 0
    // BUFFER-NEXT: 1

    Test<25, 16>(tid, 1, 6);     // M == 25, K == 16, read tile 1
    // BUFFER-NEXT: 1
}
