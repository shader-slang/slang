// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

import neural;
#pragma warning(disable: 41017)

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

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
RWStructuredBuffer<DType> inputBuffer;

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

// Initialize the weight with the pattern:
// each column is filled with -1 0 1 * column_index in a cycle.
void fillWeightMatrix<int InputSize, int OutputSize>(uint tid, bool clear = false)
{
    const int ROW = OutputSize;
    const int COL = InputSize;

    int scaleFactor = 0;
    if (ROW <= BatchSize)
    {
        if (tid > ROW)
            return;

        // InputSize mount of threads are enough to fill the input buffer
        // int index = tid * InputSize + tid;
        int row = tid;
        int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
        for (int i = 0; i < COL; i++)
        {
            int index = row * COL + i;
            if (clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(scaleFactor * (i + 1));
        }
    }
    else
    {
        int numWrites = ROW / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
            for (int j = 0; j < COL; j++)
            {
                int index = row * COL + j;
                if (clear)
                    inputBuffer[index] = DType(0);
                else
                    inputBuffer[index] = DType(scaleFactor * (j + 1));
            }
        }
    }
}

DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA, true>.Uint4AlignedK] testMatTransposeVecMul<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
{
    typealias Storage = StructuredBufferStorage<DType>;
    Storage storage = Storage(inputBuffer);

    // Construct the input vector as follow:
    // x = tid + 1
    typealias MMA = MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target, true>;
    const int OutSize = MMA.Uint4AlignedK;
    const int InSize = MMA.Uint4AlignedM;
    DType inputVector[InSize] = {};

    // Since this is transpose test, OutputSize is the actual input size.
    for (int i = 0; i < OutputSize; i++)
    {
        inputVector[i] = DType(tid + 1);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    // In this test, we can safely share the shMemB and shMemC, because they are always used at different time.
    // And they are always the same size.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType>(inputVector, ptrA, ptrB, ptrC, storage, 0);
    // serialReadA<InputSize, OutputSize>(tid);
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

// This function is just used for debugging, not for verification. So keep it here.
void serialReadA<int InputSize, int OutputSize>(uint tid)
{
    GroupMemoryBarrierWithGroupSync();

    if (tid > 0)
        return;

    for (int i = 0; i < OutputSize; i++)
    {
        printf("row: %d\n", i);
        for (int j = 0; j < InputSize; j++)
        {
            printf("%.1f ", float(inputBuffer[i * InputSize + j]));
        }
        printf("\n");
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    const int COL = InputSize;
    const int ROW = OutputSize;

    DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA, true>.Uint4AlignedK] outputVector;
    __target_switch
    {
    case cuda:
        outputVector = testMatTransposeVecMul<InputSize, OutputSize, TargetEnum.CUDA>(tid);
        break;
    case spirv:
        outputVector = testMatTransposeVecMul<InputSize, OutputSize, TargetEnum.SPIR_V>(tid);
        break;
    }

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < COL; i++)
    {
        DType expected = DType(scaleFactor * (i + 1.0f) * (tid + 1.0f));
        if (outputVector[i] != DType(expected))
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
