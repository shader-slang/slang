// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

import neural;
#pragma warning(disable: 41017)

// This test verifies that the tiled MMA load operations work correctly.

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT:ubuffer(stride=4, count=4096):name=inputBuffer
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
            inputBuffer[index] = DType(0);
        else
            inputBuffer[index] = DType(1);
    }
    else
    {
        int numWrites = InputSize / BatchSize;
        for (int i = 0; i < numWrites; i++)
        {
            int row = tid + i * BatchSize;
            int index = row * InputSize + row;
            if (clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(1);
        }
    }
}

DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA>.Uint4AlignedM] testMatVecMul<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
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

    // shared memory B and C can be reused.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryB[0]);

    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, false>( inputVector, ptrA, ptrB, ptrC, storage, 0, none, none);

    return res;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<T: __BuiltinFloatingPointType>(uint tid, SPtr sharedMem, uint rowOrColumnCount)
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
                if (sizeof(T) == 2)
                {
                    uint16_t a = (uint16_t)(value & 0xFFFF);
                    uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);


                    half aa = bit_cast<half>(a);
                    half bb = bit_cast<half>(b);
                    // debugBuffer[linearIndex++] = aa;
                    // debugBuffer[linearIndex++] = bb;
                    printf("%.1f %.1f ", float(aa), float(bb));
                }
                else
                {
                    printf("%x ", value);
                }
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
    for (int i = 0; i < OutputSize; i++)
    {
        if (outputVector[i] != DType(tid + 1))
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
