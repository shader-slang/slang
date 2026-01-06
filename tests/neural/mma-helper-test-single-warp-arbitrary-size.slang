// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)

import neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif


// Make the weight matrix a 64x64 matrix in row major order
// TEST_INPUT:ubuffer(data=[0], stride=4, count=4096):name=inputBuffer
RWStructuredBuffer<DType> inputBuffer;

// TEST_INPUT:ubuffer(data=[0], stride=4, count=64):name=biasBuffer
RWStructuredBuffer<DType> biasBuffer;

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
            inputBuffer[index] = DType(0.5);
        }
    }

    if (tid == 0)
    {
        for (int i = 0; i < 64; i++)
        {
            biasBuffer[i] = DType(i);
        }
    }
}


DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target>.Uint4AlignedM] testMatVecMul<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
{
    typealias Storage = StructuredBufferStorage<DType>;
    Storage storage = Storage(inputBuffer);

    // Construct the input vector as follow:
    // x = tid + 1
    typealias MMA = MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target, false>;
    const int InSize = MMA.Uint4AlignedK;
    const int OutSize = MMA.Uint4AlignedM;
    DType inputVector[InSize] = {};

    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = DType(tid + 1);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    // In this test, we can safely share the shMemB and shMemC, because they are always used at different time.
    // And they are always the same size.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

#if !TEST_BIAS
    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, false>(inputVector, ptrA, ptrB, ptrC, storage, 0, none, none);
#else
    Storage biasStorage = Storage(biasBuffer);
    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, true>(inputVector, ptrA, ptrB, ptrC, storage, 0,
                                                                                      biasStorage, Optional<Storage.Address>(0));
#endif
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
    DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA>.Uint4AlignedM] outputVector;
    __target_switch
    {
    case cuda:
        outputVector = testMatVecMul<InputSize, OutputSize, TargetEnum.CUDA>(tid);
        break;
    case spirv:
        outputVector = testMatVecMul<InputSize, OutputSize, TargetEnum.SPIR_V>(tid);
        break;
    }

    bool res = true;
    if (tid < OutputSize)
    {
        for (int i = 0; i < OutputSize; i++)
        {
#if !TEST_BIAS
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5);
#else
            DType expected = DType(tid + 1) * DType(InputSize) * DType(0.5) + DType(i);
#endif
            if (!equals(outputVector[i], expected))
            {
                res = false;
                break;
            }
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
    initBias<DType, 64>(tid, biasBuffer);

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
