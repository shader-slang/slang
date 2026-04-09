
// Tiled-layout outerProductAccumulate test (TiledMMAHelper path).
// Result is stored in 16x16 tiled layout instead of linear row-major.
// Sizes must be multiples of 16.

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=16
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=32
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=64

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=16
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=32
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_SIZE=64

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_SIZE=16
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_SIZE=32
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_SIZE=64

#pragma warning(disable: 41017)
import slang.neural;
import common;

// TEST_INPUT: ubuffer(data=[0], stride=4, count=1):out, name=resultBuffer
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
static const int TotalShMemSizeB = ShMemSizeB * workgroupCount * sizeof(DType)/sizeof(half);

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (TotalShMemSizeA + TotalShMemSizeB) * sizeof(uint4);
}

static const int TileSize = 16;

int tiledOffset<int NTileCols>(int row, int col)
{
    int tileRow = row / TileSize;
    int tileCol = col / TileSize;
    int localRow = row % TileSize;
    int localCol = col % TileSize;
    return (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
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

    neural_unittest.testOuterProductAccumulateTiled<DType, InputSize, OutputSize, TestShMemSize, workgroupCount>(inputData, dOutData, outputBuffer);

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    bool res = true;
    int numIterPerThread = (OutputSize + SubgroupSize - 1) / SubgroupSize;
    float scaleFactor = 1.0f / WorkgroupSize;
    scaleFactor *= scaleFactor;
    scaleFactor = scaleFactor * ((1 + WorkgroupSize) * WorkgroupSize / 2.0f);

    DType errorThreshold = DType(WorkgroupSize * 0.016);

    static const int NTileCols = InputSize / TileSize;

    for (int i = 0; i < numIterPerThread; i++)
    {
        int rowIdx = i * SubgroupSize + tid;
        if (rowIdx >= OutputSize)
            break;

        int startVal = OutputSize - rowIdx;

        for (int j = 0; j < InputSize; j++)
        {
            DType expected = DType(startVal * scaleFactor * (j + 1));
            DType actual = outputBuffer[tiledOffset<NTileCols>(rowIdx, j)];
            if (abs(expected - actual) > errorThreshold)
            {
                res = false;
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
    setBufferOneValue<TEST_SIZE * TEST_SIZE, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
    test<TEST_SIZE, TEST_SIZE>(tid, 0);
    // BUFFER: 1
}
