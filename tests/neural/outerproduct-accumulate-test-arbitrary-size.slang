
// Disable the test on CI due to stability issues.
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry computeMain -stage compute -DTEST_HALF=1 -experimental-feature

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import neural;
import common;

// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;


#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: ubuffer(stride=4, count=4096):name=outputBuffer
RWStructuredBuffer<DType> outputBuffer;

typealias BufferStorage = StructuredBufferStorage<DType>;

static const int SubgroupSize = 32;
static const int WorkgroupSize = 128;
static const int workgroupCount = WorkgroupSize / SubgroupSize;

static const int MaxK = 64;
static const int MaxM = 64;

static const int ShMemSizeA = MaxM * 2;
static const int ShMemSizeB = MaxK * 2;
static const int TotalShMemSizeA = ShMemSizeA * workgroupCount;
static const int TotalShMemSizeB = ShMemSizeB * workgroupCount * (sizeof(DType) / sizeof(half));

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (TotalShMemSizeA + TotalShMemSizeB) * sizeof(uint4);
}
typealias TestPool = SharedMemoryPool<TestShMemSize>;
static const uint OffsetA = 0;
static const uint OffsetB = TotalShMemSizeA;

// Basic test on MatMul without bias, this test covers both forward and backward pass
void TestOuterProductAccumulate<int InputSize, int OutputSize, TargetEnum Target>(uint tid)
{
    BufferStorage storage = BufferStorage(outputBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target, TestPool>;
    const int OutSize = MMA.Uint4AlignedM;
    const int InSize = MMA.Uint4AlignedK;

    DType dOutVector[OutSize] = {};
    DType inputVector[InSize] = {};

    float scaleFactor = 1.0f / WorkgroupSize;

    for (int i = 0; i < InputSize; i++)
    {
        inputVector[i] = DType((i + 1) * (tid + 1) * scaleFactor);
    }

    for (int i = 0; i < OutputSize; i++)
    {
        dOutVector[i] = DType((OutputSize - i) * scaleFactor);
    }

    // perform dOut OPA Input
    MMA.outerProductAccumulate<DType, BufferStorage, DType, DType[OutSize], DType, DType[InSize]>(OffsetA, OffsetB, dOutVector, inputVector, storage, 0);

    // serialRead<48>(tid, OffsetB);
    // serialRead<OutputSize, InputSize>(tid, outputBuffer);
}

// This function is just used for debugging, not for verification. So keep it here.
void serialRead<int Stride, T: __BuiltinFloatingPointType>(uint tid, uint sharedMemOffset)
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
            uint4 values = TestPool.data[sharedMemOffset + id * strideInVector + i];
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
                    printf("%f ", bit_cast<float>(value));
                }
            }
        }
        printf("\n");
    }
}

void serialRead<int Row, int Column>(uint tid, RWStructuredBuffer<DType> outputBuffer)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    for (int i = 0; i < Row; i++)
    {
        printf("row: %d\n", i);
        for (int j = 0; j < Column; j++)
        {
            DType value = outputBuffer[i * Column + j];
            printf("%.1f ", value);
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
    DType errorThreshold = DType(WorkgroupSize * 0.016);

    for (int i = 0; i < numIterPerThread; i++)
    {
        int rowIdx = i * SubgroupSize + tid;
        if (rowIdx >= OutputSize)
            break;

        int startVal = OutputSize - rowIdx;

        // each thread will check one row
        for (int j = 0; j < InputSize; j++)
        {
            DType expected = DType(startVal * scaleFactor * (j + 1));
            DType actual = outputBuffer[rowIdx * InputSize + j];
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

[shader("compute")]
[numthreads(WorkgroupSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        setBufferOneValue<3 * 15, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<3, 15>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER: 1

    {
        setBufferOneValue<20 * 30, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<20, 30>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1

    {
        setBufferOneValue<49 * 17, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<49, 17>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1

    {
        setBufferOneValue<33 * 9, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<33, 9>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1
}

//SPV: OpEntryPoint
