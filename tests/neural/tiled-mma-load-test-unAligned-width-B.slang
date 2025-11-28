// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=19
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=19

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=3
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=3

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTHREAD_COUNT=31
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=31

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled matrix load operations work correctly when the width of the matrix B
// is not aligned with the tile width with 0 paddings.
// Notice that the width of matrix B is actually deterrmined by the number of threads in the subgroup, therefore
// if it's not aligned with the tile width, we're going to reuse the active threads to paddings those extra columns.

// Given this fact, this test is actually testing the case that if an incomplete warp is launched, the tiled matrix load
// operations work correctly for matrix B.


// Construct the thread local vector as follow:
// x = tid * 32 + 0, 1, 2, 3 ... 31

// We will test different width of the matrix by launching different number of threads that are smaller than 32.
// For example, if we launch 3 threads, the matrix will be:
// Column   0    1     2   3  ... 15
// Row 0    0    32    64  0  ... 0
// Row 1    1    33    65  0  ... 0
// Row 2    2    34    66  0  ... 0
// ...
// Row 16   16   48    80  0  ... 0

// Explanation:
// Because there are only 3 threads launched, we can only load 3 input vectors into the tiled matrix B,
// so the matrix will be:
//
//  0    1   2   3   ... 15   -> column 0
//  32   33  34  35  ... 47   -> column 1
//  64   65  66  67  ... 80   -> column 2

// But the tile matrix B has alignment requirement, so we will pad the remaining columns with 0.
// So the matrix will be:
//
//  0    1   2   3   ... 15   -> column 0
//  32   33  34  35  ... 47   -> column 1
//  64   65  66  67  ... 80   -> column 2
//  0    0   0   0   ... 0    -> columns 3-15, 13 zeros padding



// Make the weight matrix as 32x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// InputSize is the K dimension of the matrix A, make it unaligned to test the padding logic.
static const int OutputSize = 32;
static const int SubgroupSize = THREAD_COUNT;
static const int InputSize = 32;

// Tile A is size of 32 * 32 bytes: TileHeight is at least 16.
groupshared uint4 s_sharedMemoryB[32 * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void initSharedMemoryB(uint tid)
{
    // Initialize the shared memory with all 1.0h.
    uint activeThreadCount = WaveActiveCountBits(true);
    uint numIters = (32 * 2 + activeThreadCount - 1) / activeThreadCount;
    for (int i = 0; i < numIters; i++)
    {
        uint index = tid * numIters + i;
        if (index >= 32 * 2)
            break;

        s_sharedMemoryB[index] = uint4(0x3C003C00);
    }
    GroupMemoryBarrierWithWaveSync();
}

void testLoadShB(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    half inputVector[InputSize];
    for (int i = 0; i < 32; i++)
    {
        inputVector[i] = half(tid * 32 + i);
    }

    SPtr sharedMemoryB = __getAddress(s_sharedMemoryB[0]);
    MMAHelper<half, InputSize, OutputSize, SubgroupSize>.LoadSharedMemoryFromLocalVector(sharedMemoryB, tileIndex, inputVector);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput(uint tid, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
    // So each thread will check 2 uint4 elements (32 bytes/16 half) in the shared memory.
    half expected = half(tid * 32 + tileIndex * 16);
    bool res = true;
    uint index = tid * 16;  // 16 half per thread
    uint alignedN = ((SubgroupSize + 16 - 1) / 16) * 16;

    // Because of our setting, the tid will never by larger than alignedN
    for (int i = 0; i < 2; i++)
    {
        uint indexInTile = tid * 2 + i;
        uint4 values = sharedMem[indexInTile];
        uint4 element = values;

        for (int j = 0; j < 4; j++)
        {
            uint value = element[j];
            uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
            uint16_t b = (uint16_t)(value & 0xFFFF);

            half aa = bit_cast<half>(a);
            half bb = bit_cast<half>(b);

            if (aa != expected)
            {
                return false;
            }

            expected += 1.0h;
            if (bb != expected)
            {
                return false;
            }
            expected += 1.0h;
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
            uint columnIndex = startColumn + tid;
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
            uint columnIndex = startColumn + tid;
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

void Test(uint tid, uint tileIndex, int resIndex)
{
    initSharedMemoryB(tid);

    testLoadShB(tid, tileIndex);
    // serialRead(tid, __getAddress(s_sharedMemoryB[0]));

    bool res = verifiedOutput(tid, __getAddress(s_sharedMemoryB[0]), tileIndex);
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
    if (tid >= SubgroupSize)
        return;

    Test(tid, 0, 0);    // arbitrary case, N = THREAD_COUNT, read tile 0
    // BUFFER: 1

    Test(tid, 1, 1);    // arbitrary case, N = THREAD_COUNT, read tile 1
    // BUFFER-NEXT: 1
}
