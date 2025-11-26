// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly when the width of the matrix
// is not aligned with the tile width. The result of the tile will always be aligned with 16 each row
// with 0 paddings.

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

// We will test different widths of the matrix, for example, if the width == 27,
// and we will read the 2nd tile.
//
//  16   17   18 ... 26 [5 zeros]
//  48   33   34 ... 46 [5 zeros]
//  ...
// The reason is that the 2nd tile is starting from the 17th column (index 16), and
// it has 11 elements each row, the remaining 5 elements will be padded with 0.

// Because in this test, we always using 1024 length buffer as the input buffer.
// So when K is smaller than 16, we will interpret it as 64 x 16 matrix.
// When K is larger than 16, we will interpret it as 32 x 32 matrix.
// This will impact the output verification logic.


// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void initWeightMatrix(uint tid)
{
    inputBuffer[tid] = half(tid);
}

// InputSize is the K dimension of the matrix A, make it unaligned to test the padding logic.
static const int InputSize = 27;
static const int SubgroupSize = 32;

// Tile A is size of 32 * 32 bytes: TileHeight is at least 16.
groupshared uint4 s_sharedMemoryA[32 * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA<int M, int K>(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, K, M, SubgroupSize>.LoadShA<Storage>(sharedMemoryA, tileIndex, storage, 0);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M, int K>(uint tid, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    uint rowStride = (K <= 16 ? 16 : 32);
    half expected = half(tid * rowStride + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread
    for (int i = 0; i < 2; i++)
    {
        uint indexInTile = tid * 2 + i;
        uint4 values = sharedMem[indexInTile];
        uint4 element = values;
        if (indexInTile / 2 >= M)
        {
            // Check out-of-boundary for Y direction.
            if (!values.equals(uint4(0, 0, 0, 0)))
            {
                return false;
            }
            continue;
        }

        for (int j = 0; j < 4; j++)
        {
            uint xIndex = (i * 8 + j * 2) + tileIndex * 16;

            uint value = element[j];
            if (xIndex >= K)
            {
                if (value != 0)
                {
                    return false;
                }
                continue;
            }

            uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
            uint16_t b = (uint16_t)(value & 0xFFFF);

            half aa = bit_cast<half>(a);
            half bb = bit_cast<half>(b);

            if (aa != expected)
            {
                return false;
            }

            expected += 1.0h;
            xIndex++;

            if (xIndex >= K)
            {
                if (bb != 0.0h)
                {
                    return false;
                }
                continue;
            }

            if (bb != expected)
            {
                return false;
            }
            expected += 1.0h;
        }
    }

    return true;
}

void Test<int M, int K>(uint tid, uint tileIndex, int resIndex)
{
    testLoadShA<M, K>(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput<M, K>(tid, __getAddress(s_sharedMemoryA[0]), tileIndex);
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

[numthreads(1024, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initWeightMatrix(tid);

    if (tid >= SubgroupSize)
        return;

    Test<3, 19>(tid, 1, 0);
    // BUFFER: 1

    Test<5, 32>(tid, 1, 1);      // boundary case, K = 32
    // BUFFER-NEXT: 1

    Test<14, 13>(tid, 0, 2);
    // BUFFER-NEXT: 1

    Test<25, 16>(tid, 0, 3);     // boundary case, K = 16
    // BUFFER-NEXT: 1
}
