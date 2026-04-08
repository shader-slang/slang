// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set weightBuffer = ubuffer(data=[0], stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle weightBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(data=[0], stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasBuffer;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 64;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 512) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

// Clear the weight buffer region used by a PaddedSize x PaddedSize tiled matrix.
void clearWeightBuffer<int PaddedSize>(uint tid)
{
    static const int TotalElements = PaddedSize * PaddedSize;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            weightBuffer[index] = DType(0);
    }
}

// Write an identity matrix in tiled layout.
// Tiled layout addressing for element (row, col):
//   tileRow   = row / 16
//   tileCol   = col / 16
//   NTileCols = PaddedSize / 16
//   tileIndex = tileRow * NTileCols + tileCol
//   offset    = tileIndex * 256 + (row % 16) * 16 + (col % 16)
void identityWeightMatrixTiled<int InputSize, int PaddedSize, bool Clear>(uint tid)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedSize / TileSize;

    if (Clear)
    {
        clearWeightBuffer<PaddedSize>(tid);
        return;
    }

    clearWeightBuffer<PaddedSize>(tid);
    AllMemoryBarrierWithGroupSync();

    static const int NumDiagIters = (InputSize + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumDiagIters; iter++)
    {
        int row = iter * BatchSize + tid;
        if (row >= InputSize)
            break;

        int tileRow = row / TileSize;
        int tileCol = row / TileSize;
        int localRow = row % TileSize;
        int localCol = row % TileSize;
        int tileIndex = tileRow * NTileCols + tileCol;
        int offset = tileIndex * TileSize * TileSize + localRow * TileSize + localCol;
        weightBuffer[offset] = DType(1);
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 2, false>(
        tid, weightBuffer, Optional<RWStructuredBuffer<DType>.Handle>(biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 2, false>(
        tid, weightBuffer, none);
#endif

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
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    // 4x4 (padded to 16x16)
    {
        identityWeightMatrixTiled<4, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<4, 4>(tid, 0);
        AllMemoryBarrierWithGroupSync();
        identityWeightMatrixTiled<4, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // 8x8 (padded to 16x16)
    {
        identityWeightMatrixTiled<8, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<8, 8>(tid, 1);
        AllMemoryBarrierWithGroupSync();
        identityWeightMatrixTiled<8, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // 16x16 (already aligned)
    {
        identityWeightMatrixTiled<16, 16, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 2);
        AllMemoryBarrierWithGroupSync();
        identityWeightMatrixTiled<16, 16, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // 32x32 (already aligned)
    {
        identityWeightMatrixTiled<32, 32, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 3);
        AllMemoryBarrierWithGroupSync();
        identityWeightMatrixTiled<32, 32, true>(tid);
        AllMemoryBarrierWithGroupSync();
    }

    // 64x64 (already aligned)
    {
        identityWeightMatrixTiled<64, 64, false>(tid);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 4);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
