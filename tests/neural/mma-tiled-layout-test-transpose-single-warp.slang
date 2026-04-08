// Tiled-layout MMA transpose test (single warp).
// Tests TiledMMAHelper with TransposeA=true: computes W^T * x where W is in tiled layout.
// Sizes must be multiples of 16.

// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int TileSize = 16;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

// Write weight matrix in tiled layout.
// W is OutputSize x InputSize (M x K). In tiled layout:
//   tile(tileRow, tileCol) at tileIndex * 256 + localRow * 16 + localCol
// Fill pattern: row r gets scaleFactor(r) * (col + 1) where scaleFactor cycles -1, 0, 1.
void fillWeightMatrixTiled<int InputSize, int OutputSize, int PaddedInputSize, int PaddedOutputSize, bool Clear>(uint tid)
{
    static const int NTileCols = PaddedInputSize / TileSize;
    static const int TotalElements = PaddedOutputSize * PaddedInputSize;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    if (Clear)
    {
        for (int iter = 0; iter < NumIters; iter++)
        {
            int idx = iter * BatchSize + tid;
            if (idx < TotalElements)
                inputBuffer[idx] = DType(0);
        }
        return;
    }

    for (int iter = 0; iter < NumIters; iter++)
    {
        int idx = iter * BatchSize + tid;
        if (idx < TotalElements)
            inputBuffer[idx] = DType(0);
    }
    AllMemoryBarrierWithGroupSync();

    static const int NumRowIters = (OutputSize + BatchSize - 1) / BatchSize;
    for (int iter = 0; iter < NumRowIters; iter++)
    {
        int row = iter * BatchSize + tid;
        if (row >= OutputSize)
            break;

        int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
        int tileRow = row / TileSize;
        int localRow = row % TileSize;

        for (int col = 0; col < InputSize; col++)
        {
            int tileCol = col / TileSize;
            int localCol = col % TileSize;
            int offset = (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
            inputBuffer[offset] = DType(scaleFactor * (col + 1));
        }
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, true>(
        tid, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        DType expected = DType(scaleFactor * (i + 1.0f) * (tid + 1.0f));
        if (!equals(output[i], expected))
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
        fillWeightMatrixTiled<16, 16, 16, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 0);
        fillWeightMatrixTiled<16, 16, 16, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<32, 32, 32, 32, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 1);
        fillWeightMatrixTiled<32, 32, 32, 32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<64, 64, 64, 64, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 2);
        fillWeightMatrixTiled<64, 64, 64, 64, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
