//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
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

// TEST_INPUT:ubuffer(data=[0], stride=4, count=64):name=biasBuffer
RWStructuredBuffer<DType> biasBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// // TEST_INPUT:ubuffer(stride=4, count=64):out, name=resultBuffer
//RWStructuredBuffer<uint> resultBuffer;

// // TEST_INPUT:ubuffer(stride=4, count=1024):out, name=debugBuffer
// RWStructuredBuffer<DType> debugBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

// Allocate large enough shared memory in this test.
groupshared bool s_resultMem[BatchSize/32];

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}
typealias TestPool = SharedMemoryPool<TestShMemSize>;
static const uint OffsetA = 0;
static const uint OffsetB = 256;

// Initialize the weight matrix as identity matrix.
void identityWeightMatrix<int InputSize, bool Clear>(uint tid)
{
    if (InputSize <= BatchSize)
    {
        if (tid > InputSize)
            return;

        // InputSize mount of threads are enough to fill the input buffer
        int index = tid * InputSize + tid;
        if (Clear)
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
            if (Clear)
                inputBuffer[index] = DType(0);
            else
                inputBuffer[index] = DType(1);
        }
    }
}

DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA, TestPool>.Uint4AlignedM] testMatVecMul<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
{
    typealias Storage = StructuredBufferStorage<DType>;
    Storage storage = Storage(inputBuffer);

    // Construct the input vector as follow:
    // x = tid + 1
    typealias MMA = MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target, TestPool, false>;
    const int InSize = MMA.Uint4AlignedK;
    const int OutSize = MMA.Uint4AlignedM;
    DType inputVector[InSize] = {};
    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = DType(tid + 1);
    }

#if !TEST_BIAS
    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, false>(inputVector, OffsetA, OffsetB, OffsetB, storage, 0, none, none);
#else
    Storage biasStorage = Storage(biasBuffer);
    let res = MMA.mma<DType, Storage, DType[InSize], DType[OutSize], DType, true>(inputVector, OffsetA, OffsetB, OffsetB, storage, 0,
                                                                                    biasStorage, Optional<Storage.Address>(0));
#endif

    // serialRead<DType>(tid, ptrC, 32);
    return res;
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<T: __BuiltinFloatingPointType>(uint tid, uint sharedMemOffset, uint rowOrColumnCount)
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
            uint4 values = TestPool.data[sharedMemOffset + id * numVectors + i];
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
#if !TEST_BIAS
        DType expected = DType(tid + 1);
#else
        DType expected = DType(tid + 1) + DType(i);
#endif
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }
    return res;
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    DType[MMAHelper<DType, InputSize, OutputSize, SubgroupSize, TargetEnum.CUDA, TestPool>.Uint4AlignedM] outputVector;

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
    bool finalResult = collectResults<BatchSize>(tid, res);
    if (tid == 0)
    {
        outputBuffer[resIndex] = finalResult ? 1 : 0;
    }
}


[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64>(tid, biasBuffer);

    {
        identityWeightMatrix<4, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<4, 4>(tid, 0);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<4, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<8, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<8, 8>(tid, 1);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<8, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<16, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<16, 16>(tid, 2);

        AllMemoryBarrierWithGroupSync();
        identityWeightMatrix<16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<32, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<32, 32>(tid, 3);
        AllMemoryBarrierWithGroupSync();

        identityWeightMatrix<32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        identityWeightMatrix<64, false>(tid);
        AllMemoryBarrierWithGroupSync();

        test<64, 64>(tid, 4);
    }


    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
