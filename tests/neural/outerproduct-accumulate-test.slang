
// On Vulkan, we can only test float type for now because atomicAdd is not supported for half on our CI machine.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1
import neural;
#pragma warning(disable: 41017)


// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;


#if TEST_HALF
typealias StorageElemType = half;
#else
typealias StorageElemType = float;
#endif

// TEST_INPUT: ubuffer(stride=4, count=4096):name=outputBuffer
RWStructuredBuffer<StorageElemType> outputBuffer;

typealias BufferStorage = StructuredBufferStorage<StorageElemType>;

static const int SubgroupSize = 32;
static const int WorkgroupSize = 128;
static const int workgroupCount = WorkgroupSize / SubgroupSize;

static const int MaxK = 64;
static const int MaxM = 64;

static const int ShMemSizeA = MaxM * 2;
static const int ShMemSizeB = MaxK * 2;

groupshared uint4 s_sharedMemoryA[ShMemSizeA * workgroupCount];
groupshared uint4 s_sharedMemoryB[ShMemSizeB * workgroupCount];

typealias SPtr = Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared>;

// Basic test on MatMul without bias, this test covers both forward and backward pass
void TestOuterProductAccumulate<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
{
    BufferStorage storage = BufferStorage(outputBuffer);

    typealias MMA = MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target>;
    const int OutSize = MMA.Uint4AlignedM;
    const int InSize = MMA.Uint4AlignedK;

    half dOutVector[OutSize] = {};
    half inputVector[InSize] = {};

    float scaleFactor = 1.0f / WorkgroupSize;

    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = half((i + 1) * (tid + 1) * scaleFactor);
    }

    for (int i = 0; i < OutputSize; i++)
    {
        dOutVector[i] = half((OutputSize - i) * scaleFactor);
    }

    SPtr ptrA = __getAddress(s_sharedMemoryA[0]);
    SPtr ptrB = __getAddress(s_sharedMemoryB[0]);

    // perform dOut OPA Input
    MMA.outerProductAccumulate<StorageElemType, BufferStorage, half, half[OutSize], half, half[InSize]>(ptrA, ptrB, dOutVector, inputVector, storage, 0);

    // serialRead<48>(tid, ptrB);
    // serialRead<OutputSize, InputSize>(tid, outputBuffer);
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<int Stride>(uint tid, SPtr sharedMem)
{
    GroupMemoryBarrierWithGroupSync();

    if (tid > 0)
        return;

    for (int id = 0; id < 16; id++)
    {
        printf("tid: %d\n", id);
        int strideInVector  = Stride / (sizeof(uint4) / sizeof(half));
        for (int i = 0; i < strideInVector; i++)
        {
            uint4 values = sharedMem[id * strideInVector + i];
            uint4 element = values;
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

void serialRead<int Row, int Column>(uint tid, RWStructuredBuffer<StorageElemType> outputBuffer)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    for (int i = 0; i < Row; i++)
    {
        for (int j = 0; j < Column; j++)
        {
            float value = outputBuffer[i * Column + j];
            printf("%.4f ", value);
        }
        printf("\n");
    }
}

void test<int InputSize, int OutputSize>(uint tid, uint resIndex)
{
    __target_switch
    {
    case cuda:
        TestOuterProductAccumulate<InputSize, OutputSize, TargetEnum.CUDA>(tid);
        break;
    case spirv:
        TestOuterProductAccumulate<InputSize, OutputSize, TargetEnum.SPIR_V>(tid);
        break;
    }

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    // We just use the first warp to check the result to simplicity
    bool res = true;
    int numIterPerThread = (OutputSize + SubgroupSize - 1) / SubgroupSize;
    float scaleFactor = 1.0f / WorkgroupSize;
    scaleFactor *= scaleFactor;
    scaleFactor = scaleFactor * ((1 + WorkgroupSize) * WorkgroupSize/2.0f);

    // Note that half type is not precise, and more accumulate will cause more error, so
    // we increase the error threshold by using the scale of the workgroup size which is exactly
    // the number of accumulations.
    // This is a purely empirical value, good input pattern could make the error smaller.
    StorageElemType errorThreshold = StorageElemType(WorkgroupSize * 0.016);

    for (int i = 0; i < numIterPerThread; i++)
    {
        int rowIdx = i * SubgroupSize + tid;
        if (rowIdx >= OutputSize)
            break;

        int startVal = OutputSize - rowIdx;

        // each thread will check one row
        for (int j = 0; j < InputSize; j++)
        {
            StorageElemType expected = StorageElemType(startVal * scaleFactor * (j + 1));
            StorageElemType actual = outputBuffer[rowIdx * InputSize + j];
            if (abs(expected - actual) > errorThreshold)
            {
                res = false;
                printf("tid: %d, rowIdx: %d, j: %d, expected: %.4f, actual: %.4f\n", tid, rowIdx, j, float(expected), float(actual));
                break;
            }
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        resultBuffer[resIndex] = res ? 1 : 0;
}

void cleanOutputBuffer<int InputSize, int OutputSize>(uint tid)
{
    int size = OutputSize * InputSize;
    int numIter = (size + WorkgroupSize - 1) / WorkgroupSize;
    for (int i = 0; i < numIter; i++)
    {
        int index = i * WorkgroupSize + tid;
        if (index >= size)
            break;

        outputBuffer[index] = StorageElemType(0.0f);
    }
    AllMemoryBarrierWithGroupSync();
}

[shader("compute")]
[numthreads(WorkgroupSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        cleanOutputBuffer<8, 8>(tid);
        test<8, 8>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer<16, 16>(tid);
        test<16, 16>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer<32, 32>(tid);
        test<32, 32>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer<64, 64>(tid);
        test<64, 64>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
