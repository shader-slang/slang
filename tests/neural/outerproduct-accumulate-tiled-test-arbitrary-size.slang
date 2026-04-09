
// Tiled-layout outerProductAccumulate test with arbitrary (non-tile-aligned) sizes.
// InputSize/OutputSize are padded to the next multiple of 16 for TiledMMAHelper,
// but only the valid region is verified.

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
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    float sf = 1.0f / float(WorkgroupSize);
    DType inputData[PaddedInputSize];
    for (int i = 0; i < PaddedInputSize; i++)
        inputData[i] = DType((i + 1) * (tid + 1) * sf);
    DType dOutData[PaddedOutputSize];
    for (int i = 0; i < PaddedOutputSize; i++)
        dOutData[i] = DType((PaddedOutputSize - i) * sf);

    neural_unittest.testOuterProductAccumulateTiled<DType, PaddedInputSize, PaddedOutputSize, TestShMemSize, workgroupCount>(inputData, dOutData, outputBuffer);

    int subgroupIndex = tid / SubgroupSize;
    if (subgroupIndex != 0)
        return;

    bool res = true;
    int numIterPerThread = (OutputSize + SubgroupSize - 1) / SubgroupSize;
    float scaleFactor = 1.0f / WorkgroupSize;
    scaleFactor *= scaleFactor;
    scaleFactor = scaleFactor * ((1 + WorkgroupSize) * WorkgroupSize / 2.0f);

    DType errorThreshold = DType(WorkgroupSize * 0.016);

    static const int NTileCols = PaddedInputSize / TileSize;

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
                printf("tid: %d, rowIdx: %d, j: %d, expected: %.4f, actual: %.4f\n", tid, rowIdx, j, float(expected), float(actual));
                break;
            }
        }
    }

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        resultBuffer[resIndex] = res ? 1 : 0;
}

void clearTiledBuffer<int PaddedSize, int WorkgroupSize>(uint tid)
{
    static const int TotalElements = PaddedSize * PaddedSize;
    setBufferOneValue<TotalElements, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
}

[shader("compute")]
[numthreads(WorkgroupSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // 3x15 → padded to 16x16
    {
        clearTiledBuffer<16, WorkgroupSize>(tid);
        test<3, 15>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER: 1

    // 20x30 → padded to 32x32
    {
        clearTiledBuffer<32, WorkgroupSize>(tid);
        test<20, 30>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1

    // 49x17 → padded to 64x32
    {
        setBufferOneValue<64 * 32, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<49, 17>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1

    // 33x9 → padded to 48x16
    {
        setBufferOneValue<48 * 16, WorkgroupSize>(tid, outputBuffer, DType(0.0f));
        test<33, 9>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }
    // BUFFER-NEXT: 1
}

//SPV: OpEntryPoint
