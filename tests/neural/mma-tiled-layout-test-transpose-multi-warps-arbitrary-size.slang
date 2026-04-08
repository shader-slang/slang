// Tiled-layout MMA transpose test with arbitrary sizes (multi warps, BatchSize=64 = 2 warps).
// Sizes are padded to multiples of 16 for TiledMMAHelper.

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

static const int BatchSize = 64;
static const int SubgroupSize = 32;
static const int TileSize = 16;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

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

    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 2, true>(
        tid, inputBuffer, none);

    bool res = true;
    int scaleFactor = (OutputSize % 3 == 0) ? 0 : -1;
    for (int i = 0; i < InputSize; i++)
    {
        int intVal = (i + 1) * (tid + 1);
        DType expected = DType(scaleFactor) * DType(intVal);
        if (!equals(output[i], expected))
        {
            res = false;
            break;
        }
    }

    bool finalResult = collectResults<BatchSize>(tid, res);
    if (tid == 0)
        outputBuffer[resIndex] = finalResult ? 1 : 0;
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    {
        fillWeightMatrixTiled<3, 5, 16, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
        fillWeightMatrixTiled<3, 5, 16, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<11, 18, 16, 32, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
        fillWeightMatrixTiled<11, 18, 16, 32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<23, 9, 32, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
        fillWeightMatrixTiled<23, 9, 32, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<35, 3, 48, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
        fillWeightMatrixTiled<35, 3, 48, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    {
        fillWeightMatrixTiled<17, 64, 32, 64, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
        fillWeightMatrixTiled<17, 64, 32, 64, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
