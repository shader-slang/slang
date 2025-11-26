// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=7
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=7

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=14
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=14

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=23
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=23

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTHREAD_COUNT=32
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTHREAD_COUNT=32

import neural;
#pragma warning(disable: 41017)


// This test verifies that if an incomplete warp is launched, the tiled MMA load operations work correctly.
// We test it by launching arbitrary number of threads that are smaller than 32.

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



// We will test different heights of the matrix, all the heights are smaller than 16.
// For example, if we set the height to 3, the matrix will be:
//
//   0    1    2 ... 15
//  32   33   34 ... 47
//  64   65   66 ... 79
//  0    0    0 ... 0
//  0    0    0 ... 0
//  [paddings for row 4-16]

// Make the weight matrix as 16x32 matrix in row major order
// TEST_INPUT:ubuffer(stride=2, count=1024):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

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

static const int InputSize = 32;
static const int SubgroupSize = 32;

// Tile A is size of 32 * 32 bytes: TileHeight is at least 16.
groupshared uint4 s_sharedMemoryA[32 * 2];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

void testLoadShA<int M>(uint tid, uint tileIndex)
{
    typealias Storage = StructuredBufferStorage<half>;

    Storage storage = Storage(inputBuffer);

    SPtr sharedMemoryA = __getAddress(s_sharedMemoryA[0]);
    MMAHelper<half, InputSize, M, SubgroupSize>.LoadShA<Storage>(sharedMemoryA, tileIndex, storage, 0);
    GroupMemoryBarrierWithWaveSync();
}

bool verifiedOutput<int M>(uint tid, uint size, SPtr sharedMem, uint tileIndex)
{
    // Verify the output is correct, each thread will verify one row/column of the shared memory.
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
            uint value = element[j];

            uint16_t a = (uint16_t)((value >> 16) & 0xFFFF);
            uint16_t b = (uint16_t)(value & 0xFFFF);

            half aa = bit_cast<half>(a);
            half bb = bit_cast<half>(b);

            if (indexInTile / 2 >= M)
            {
                // Checking paddings are correct for out-of-range elements.
                if (aa != 0.0h || bb != 0.0h)
                {
                    return false;

                }
            }
            else
            {
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
    }

    return true;
}

void Test<int M>(uint tid, int resIndex)
{
    testLoadShA<M>(tid, 0);
    // serialRead(tid, __getAddress(s_sharedMemoryA[0]));

    bool res = verifiedOutput<M>(tid, M, __getAddress(s_sharedMemoryA[0]), 0);
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

    // Test for different heights of the matrix.
    Test<7>(tid, 0);
    // BUFFER: 1

    Test<16>(tid, 1);
    // BUFFER-NEXT: 1

    Test<23>(tid, 2);
    // BUFFER-NEXT: 1

    Test<32>(tid, 3);
    // BUFFER-NEXT: 1
}
