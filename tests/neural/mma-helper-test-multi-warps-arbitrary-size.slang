// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly.
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
//   Row 30: 960  961  962  963  ...  975  976  977  ...  991
//   Row 31: 992  993  994  995  ... 1007 1008 1009  ... 1023
//


// Make the weight matrix a 64x64 matrix in row major order
// TEST_INPUT:ubuffer(data=[0], stride=2, count=4096):name=inputBuffer
RWStructuredBuffer<half> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// // TEST_INPUT:ubuffer(stride=4, count=256):out, name=debugBuffer
// RWStructuredBuffer<half> debugBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

// Allocate large enough shared memory in this test.
groupshared uint4 s_sharedMemoryA[256];
groupshared uint4 s_sharedMemoryB[256];

groupshared bool s_resultMem[BatchSize/32];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;
typealias HalfPtr = Ptr<half, Access::ReadWrite, AddressSpace::GroupShared>;

// Initialize the weight matrix as half(0.5).
void fillWeightMatrix<int InputSize, int OutputSize>(uint tid)
{
    uint numIter = (OutputSize + BatchSize - 1) / BatchSize;

    for (int i = 0; i < numIter; i++)
    {
        int rowIndex = i * BatchSize + tid;
        if (rowIndex >= OutputSize)
            break;

        int startIndex = rowIndex * InputSize;
        for (int j = 0; j < InputSize; j++)
        {
            int index = startIndex + j;
            inputBuffer[index] = half(0.5);
        }
    }
}


half[MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedM] testMatVecMul<int InputSize, int OutputSize>(uint tid)
{
    typealias Storage = StructuredBufferStorage<half>;
    Storage storage = Storage(inputBuffer);

    // Construct the input vector as follow:
    // x = tid + 1
    // const int uint4AlignedInputSize = MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedK;
    half inputVector[MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedK];

    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = half(tid + 1);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    // In this test, we can safely share the shMemB and shMemC, because they are always used at different time.
    // And they are always the same size.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

    let res = MMAHelper<half, InputSize, OutputSize, SubgroupSize, false>.mma<Storage>(inputVector, ptrA, ptrB, ptrC, storage, 0);
    // serialRead(tid, ptrC, 32);
    return res;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead(uint tid, SPtr sharedMem, uint rowOrColumnCount)
{
    GroupMemoryBarrierWithGroupSync();

    if (tid > 0)
        return;

    int linearIndex = 0;
    for (int id = 0; id < rowOrColumnCount; id++)
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
                // debugBuffer[linearIndex++] = aa;
                // debugBuffer[linearIndex++] = bb;
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    let outputVector = testMatVecMul<InputSize, OutputSize>(tid);

    bool res = true;
    if (tid < OutputSize)
    {
        float expected = float(tid + 1) * float(InputSize) * 0.5f;
        for (int i = 0; i < OutputSize; i++)
        {
            if (outputVector[i] != half(expected))
            {
                res = false;
                break;
            }
        }
    }

    // each warp will check the result
    res = WaveActiveAllTrue(res);

    int subgroupIndex = tid / 32;
    int laneId = WaveGetLaneIndex();
    // use first lane of each warp to write the result to the shared memory so that
    // we can reduce the result further
    if (laneId == 0)
    {
        // Note max subgroupIndex is "BatchSize/32 - 1"
        s_resultMem[subgroupIndex] = res;
    }

    int warpCount = BatchSize / 32;
    bool finalResult = true;
    // We will use the "warpCount" threads in the first warp to reduce the result.
    if (tid < warpCount)
    {
        finalResult = s_resultMem[tid];
    }

    finalResult = WaveActiveAllTrue(finalResult);

    // Finally, use the first thread to write the result to the output buffer.
    if (tid == 0)
    {
        outputBuffer[resIndex] = finalResult ? 1 : 0;
    }
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        fillWeightMatrix<3, 5>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<3, 5>(tid, 0);
    }

    {
        fillWeightMatrix<11, 18>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<11, 18>(tid, 1);
    }

    {
        fillWeightMatrix<23, 9>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<23, 9>(tid, 2);
    }

    {
        fillWeightMatrix<35, 3>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<35, 3>(tid, 3);
    }

    {
        fillWeightMatrix<17, 64>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
