
// Disable the test on CI due to stability issues.
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry computeMain -stage compute -DTEST_HALF=1 -experimental-feature

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

// TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out, name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

//TEST_INPUT: set outputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle outputBuffer;

static const int SubgroupSize = 32;
static const int WorkgroupSize = 128;
static const int workgroupCount = WorkgroupSize / SubgroupSize;

static const int MaxK = 64;
static const int MaxM = 64;
static const int ShMemSizeA = MaxM * 4;
static const int ShMemSizeB = MaxK * 4;
static const int TotalShMemSizeA = ShMemSizeA * workgroupCount;
static const int TotalShMemSizeB = ShMemSizeB * workgroupCount * (sizeof(DType) / sizeof(half));

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (TotalShMemSizeA + TotalShMemSizeB) * sizeof(uint4);
}

void test<int InputSize, int OutputSize>(uint tid, uint resIndex)
{
    float sf = 1.0f / float(WorkgroupSize);
    DType inputData[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType((i + 1) * (tid + 1) * sf);
    DType dOutData[OutputSize];
    for (int i = 0; i < OutputSize; i++)
        dOutData[i] = DType((OutputSize - i) * sf);

    neural_unittest.testOuterProductAccumulate<DType, InputSize, OutputSize, TestShMemSize, workgroupCount>(inputData, dOutData, outputBuffer);

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    bool res = true;
    int numIterPerThread = (OutputSize + SubgroupSize - 1) / SubgroupSize;
    float scaleFactor = 1.0f / WorkgroupSize;
    scaleFactor *= scaleFactor;
    scaleFactor = scaleFactor * ((1 + WorkgroupSize) * WorkgroupSize/2.0f);

    DType errorThreshold = DType(WorkgroupSize * 0.016);

    for (int i = 0; i < numIterPerThread; i++)
    {
        int rowIdx = i * SubgroupSize + tid;
        if (rowIdx >= OutputSize)
            break;

        int startVal = OutputSize - rowIdx;

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
