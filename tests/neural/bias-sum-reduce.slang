
// On Vulkan, we can only test float type for now because atomicAdd is not supported for half on our CI machine.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1
import neural;
#pragma warning(disable: 41017)


// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;


#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: ubuffer(stride=4, count=64):name=outputBuffer
RWStructuredBuffer<DType> outputBuffer;

typealias BufferStorage = StructuredBufferStorage<DType>;

static const int SubgroupSize = 32;
static const int WorkgroupSize = 128;
static const int workgroupCount = WorkgroupSize / SubgroupSize;
static const int InputSize = 8;
static const int MaxOutputSize = 64;

typealias ShMemSize = SharedMemorySize< DType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias Layer1 = ShMemSize.OfLayer1<InputSize, MaxOutputSize>;
typealias TestPool = SharedMemoryPool<Layer1>;

// Basic test on MatMul without bias, this test covers both forward and backward pass
void TestBiasSumReduce<int OutputSize, TargetEnum Target>(uint tid)
{
    BufferStorage storage = BufferStorage(outputBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, SubgroupSize, Target, TestPool, false>;
    const int OutSize = MMA.Uint4AlignedM;
    const int InSize = MMA.Uint4AlignedK;

    DType dOutVector[OutSize] = {};

    for (int i = 0; i < OutputSize; i++)
    {
        dOutVector[i] = DType((i + 1) * 0.01);
    }

    uint subgroupId = tid / SubgroupSize;

    // perform dOut OPA Input
    MMA.sumReduceRows<DType, BufferStorage, DType[OutSize]>(0, dOutVector, subgroupId, storage, 0);

    // serialRead<16, DType>(tid, 0);
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
        int strideInVector  = Stride / (sizeof(uint4) / sizeof(T));
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
                    printf("%.2f %.2f ", float(aa), float(bb));
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

void serialRead<int Size>(uint tid, RWStructuredBuffer<DType> outputBuffer)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    for (int i = 0; i < Size; i++)
    {
        DType value = outputBuffer[i];
        printf("%.4f ", float(value));
    }
    printf("\n");
}

void test<int OutputSize>(uint tid, uint resIndex)
{
    __target_switch
    {
    case cuda:
        TestBiasSumReduce<OutputSize, TargetEnum.CUDA>(tid);
        break;
    case spirv:
        TestBiasSumReduce<OutputSize, TargetEnum.SPIR_V>(tid);
        break;
    }

    // serialRead<OutputSize>(tid, outputBuffer);

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    if (tid == 0)
    {
        bool isPassed = true;
        for (int i = 0; i < OutputSize; i++)
        {
            DType value = outputBuffer[i];
            DType expected = DType((i + 1) * 0.01 * WorkgroupSize);
            if (abs(value - expected) > DType(0.001))
            {
                isPassed = false;
                break;
            }
        }
        resultBuffer[resIndex] = isPassed ? 1 : 0;
    }
}

void cleanOutputBuffer(uint tid)
{
    for (int i = 0; i < MaxOutputSize; i++)
    {
        outputBuffer[i] = DType(0.0f);
    }
    AllMemoryBarrierWithGroupSync();
}

[shader("compute")]
[numthreads(WorkgroupSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        cleanOutputBuffer(tid);
        test<3>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer(tid);
        test<15>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer(tid);
        test<23>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    {
        cleanOutputBuffer(tid);
        test<47>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
