// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

import slang.neural;
import common;
#pragma warning(disable: 41017)

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// TEST_INPUT: set weightBuffer = ubuffer(stride=4, count=4096)
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

// Compute the tiled layout offset for element (row, col) in a matrix with PaddedK columns.
int tiledOffset<int PaddedK>(int row, int col)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedK / TileSize;
    int tileRow = row / TileSize;
    int tileCol = col / TileSize;
    int localRow = row % TileSize;
    int localCol = col % TileSize;
    int tileIndex = tileRow * NTileCols + tileCol;
    return tileIndex * TileSize * TileSize + localRow * TileSize + localCol;
}

// Clear the weight buffer region used by a PaddedM x PaddedK tiled matrix.
void clearWeightBuffer<int PaddedM, int PaddedK>(uint tid)
{
    static const int TotalElements = PaddedM * PaddedK;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            weightBuffer[index] = DType(0);
    }
}

// Fill weight matrix with 0.125 in tiled MatB(K, M) layout.
// MatB has K=InputSize rows × M=OutputSize columns, padded to PaddedK × PaddedM.
void fillWeightMatrixTiled<int InputSize, int OutputSize, int PaddedK, int PaddedM>(uint tid)
{
    clearWeightBuffer<PaddedM, PaddedK>(tid);
    AllMemoryBarrierWithGroupSync();

    static const int NumRowIters = (InputSize + BatchSize - 1) / BatchSize;

    for (int iter = 0; iter < NumRowIters; iter++)
    {
        int row = iter * BatchSize + tid;
        if (row >= InputSize)
            break;

        for (int col = 0; col < OutputSize; col++)
        {
            int offset = tiledOffset<PaddedM>(row, col);
            weightBuffer[offset] = DType(0.125f);
        }
    }

    AllMemoryBarrierWithGroupSync();
    convertTiledToNativeB<DType, PaddedK, PaddedM, BatchSize>(tid, weightBuffer);
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    DType inputData[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 2, false>(
        inputData, weightBuffer, Optional<RWStructuredBuffer<DType>.Handle>(biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 2, false>(
        inputData, weightBuffer, none);
#endif

    bool res = true;
    for (int i = 0; i < OutputSize; i++)
    {
        DType scaleFactor = DType(0.125f);
#if !TEST_BIAS
        DType expected = DType((tid + 1) * InputSize) * scaleFactor;
#else
        DType expected = DType((tid + 1) * InputSize) * scaleFactor + DType(i);
#endif
        if (!equals(output[i], expected))
        {
            if (tid == 0)
                printf("FAIL resIndex=%d i=%d expected=%f got=%f\n", resIndex, i, float(expected), float(output[i]));
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

    // 3x5 (padded to 16x16)
    {
        fillWeightMatrixTiled<3, 5, 16, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<3, 5>(tid, 0);
        AllMemoryBarrierWithGroupSync();
    }

    // 11x18 (padded to 16x32)
    {
        fillWeightMatrixTiled<11, 18, 16, 32>(tid);
        AllMemoryBarrierWithGroupSync();
        test<11, 18>(tid, 1);
        AllMemoryBarrierWithGroupSync();
    }

    // 23x9 (padded to 32x16)
    {
        fillWeightMatrixTiled<23, 9, 32, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<23, 9>(tid, 2);
        AllMemoryBarrierWithGroupSync();
    }

    // 35x3 (padded to 48x16)
    {
        fillWeightMatrixTiled<35, 3, 48, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<35, 3>(tid, 3);
        AllMemoryBarrierWithGroupSync();
    }

    // 17x64 (padded to 32x64)
    {
        fillWeightMatrixTiled<17, 64, 32, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<17, 64>(tid, 4);
    }
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
