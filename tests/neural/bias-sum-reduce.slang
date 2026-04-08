
// On Vulkan, we can only test float type for now because atomicAdd is not supported for half on our CI machine.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1
import slang.neural;
#pragma warning(disable: 41017)


// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set outputBuffer = ubuffer(stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle outputBuffer;

static const int SubgroupSize = 32;
static const int WorkgroupSize = 128;
static const int workgroupCount = WorkgroupSize / SubgroupSize;
static const int InputSize = 8;
static const int MaxOutputSize = 64;

typealias ShMemSize = SharedMemorySize< DType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias Layer1 = ShMemSize.OfLayer1<InputSize, MaxOutputSize>;

void test<int OutputSize>(uint tid, uint resIndex)
{
    DType dOut[OutputSize];
    for (int i = 0; i < OutputSize; i++)
        dOut[i] = DType((i + 1) * 0.0078125);

    neural_unittest.testBiasSumReduce<DType, InputSize, OutputSize, Layer1, workgroupCount>(tid, dOut, outputBuffer);

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    if (tid == 0)
    {
        bool isPassed = true;
        for (int i = 0; i < OutputSize; i++)
        {
            DType value = outputBuffer[i];
            DType expected = DType(i + 1);
            DType tolerance = DType(0.001);
            if (abs(value - expected) > tolerance)
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
