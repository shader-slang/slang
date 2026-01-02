//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

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
// TEST_INPUT:ubuffer(data=[0], stride=4, count=4096):name=inputBuffer
RWStructuredBuffer<DType> inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;
// TEST_INPUT:ubuffer(stride=4, count=64):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

// TEST_INPUT:ubuffer(stride=4, count=4096):name=debugBuffer
RWStructuredBuffer<DType> debugBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

// Allocate large enough shared memory in this test.
groupshared uint4 s_sharedMemoryA[256];
groupshared uint4 s_sharedMemoryB[256];

groupshared uint4 s_sharedMemoryC[512];

groupshared bool s_resultMem[BatchSize/32];

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
    // In this test, we can safely share the shMemB and shMemC, because they are always used at different time.
    // And they are always the same size.
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);
    SPtr ptrC = __getAddress(s_sharedMemoryC[0]);

    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, false>( inputVector, ptrA, ptrB, ptrC, storage, 0, none, none);

    // GroupMemoryBarrierWithGroupSync();

    // only take a look at the first warp's shared memory
    // serialRead<DType>(tid, ptrC, 32);
    return res;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<T: __BuiltinFloatingPointType>(uint tid, SPtr sharedMem, uint rowOrColumnCount)
{
    if (tid > 0)
        return;

    int linearIndex = 0;
    for (int id = 0; id < rowOrColumnCount; id++)
    {
        printf("tid: %d\n", id);
        int numVectors = sizeof(uint4) / sizeof(T);
        for (int i = 0; i < numVectors; i++)
        {
            uint4 values = sharedMem[id * numVectors + i];
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
                    printf("%.1f %.1f ", float(aa), float(bb));
                }
                else
                {
                    float aa = bit_cast<float>(value);
                    printf("%.1f ", aa);
                }
            }
        }
        printf("\n");
    }
}

bool verifyResult<int OutputSize, T: IArray<DType>>(uint tid, T output)
{
    bool res = true;
    for (int i = 0; i < OutputSize; i++)
    {
        if (output[i] != DType(tid + 1))
        {
            res = false;
            break;
        }
    }
    return res;
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

    bool res = verifyResult<OutputSize>(tid, outputVector);

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

    // Sync the shared memory write
    AllMemoryBarrierWithGroupSync();

    // We will use the "warpCount" threads in the first warp to reduce the result.
    if (subgroupIndex == 0)
    {
        if (laneId < warpCount)
        {
            finalResult = s_resultMem[tid];
        }

        finalResult = WaveActiveAllTrue(finalResult);

        if (tid == 0)
        {
            outputBuffer[resIndex] = finalResult ? 1 : 0;
        }
    }

    // if (resIndex == 4)
    // {
    //     for (int i = 0; i < OutputSize; i++)
    //     {
    //         int rowIdx = tid;
    //         int index = rowIdx * OutputSize + i;
    //         debugBuffer[index] = DType(outputVector[i]);
    //     }
    // }
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        identityWeightMatrix<4>(tid);
        AllMemoryBarrierWithGroupSync();

        test<4, 4>(tid, 0);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<4>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<8>(tid);
        AllMemoryBarrierWithGroupSync();

        test<8, 8>(tid, 1);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<8>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<16>(tid);
        AllMemoryBarrierWithGroupSync();

        test<16, 16>(tid, 2);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<16>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<32>(tid);
        AllMemoryBarrierWithGroupSync();

        test<32, 32>(tid, 3);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<32>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    {
        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<64>(tid);
        AllMemoryBarrierWithGroupSync();

        test<64, 64>(tid, 4);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
