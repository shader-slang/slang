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

static const int BatchSize = 32;
static const int SubgroupSize = 32;

// Allocate large enough shared memory in this test.
groupshared uint4 s_sharedMemoryA[256];
groupshared uint4 s_sharedMemoryB[256];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;
typealias HalfPtr = Ptr<half, Access::ReadWrite, AddressSpace::GroupShared>;

// Initialize the weight matrix as identity matrix.
void identityWeightMatrix<int InputSize>(uint tid, bool clear = false)
{
    if (InputSize <= BatchSize)
    {
        if (tid > InputSize)
            return;

        // InputSize mount of threads are enough to fill the input buffer
        int index = tid * InputSize + tid;
        if (clear)
            inputBuffer[index] = half(0);
        else
            inputBuffer[index] = half(1);
    }
    else
    {
        int numWrites = InputSize / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int index = row * InputSize + row;
            if (clear)
                inputBuffer[index] = half(0);
            else
                inputBuffer[index] = half(1);
        }
    }
}

half[MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedM] testMatVecMul<int InputSize, int OutputSize>(uint tid)
{
    typealias Storage = StructuredBufferStorage<half>;
    Storage storage = Storage(inputBuffer);

    // Construct the input vector as follow:
    // x = tid + 1
    const int InSize = MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedK;
    const int OutSize = MMAHelper<half, InputSize, OutputSize, SubgroupSize>.Uint4AlignedM;
    half inputVector[InSize];
    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = half(tid + 1);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    // In this test, we can safely share the shMemB and shMemC, because they are always used at different time.
    // And they are always the same size.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

    let res = MMAHelper<half, InputSize, OutputSize, SubgroupSize, false
                        >.mma<half, Storage, half[InSize], half[OutSize]
                             >(inputVector, ptrA, ptrB, ptrC, storage, 0);
    // serialRead(tid, ptrB, 16);
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
    for (int i = 0; i < OutputSize; i++)
    {
        if (outputVector[i] != half(tid + 1))
        {
            res = false;
            break;
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        identityWeightMatrix<4>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<4, 4>(tid, 0);
        identityWeightMatrix<4>(tid, true);
    }

    {
        identityWeightMatrix<8>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<8, 8>(tid, 1);
        identityWeightMatrix<8>(tid, true);
    }

    {
        identityWeightMatrix<16>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<16, 16>(tid, 2);
        identityWeightMatrix<16>(tid, true);
    }

    {
        identityWeightMatrix<32>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<32, 32>(tid, 3);
        identityWeightMatrix<32>(tid, true);
    }

    {
        identityWeightMatrix<64>(tid);
        GroupMemoryBarrierWithWaveSync();
        test<64, 64>(tid, 4);
        identityWeightMatrix<64>(tid, true);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
