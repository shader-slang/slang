//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0
// VK half disabled: backward pass uses half atomics for bias/weight gradient accumulation,
// which requires VK_EXT_shader_atomic_float16 not available on our test GPUs.
// DISABLED-TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature -xslang -DTEST_HALF=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// Weight matrix in tiled layout (read by transpose MMA)
// 2x size needed: convertTiledToNativeB uses the second half as scratch space.
// TEST_INPUT: set weightBuffer = ubuffer(stride=4, count=8192)
uniform RWStructuredBuffer<DType>.Handle weightBuffer;

// Weight gradient (written by outer product)
// TEST_INPUT: set weightGradBuffer = ubuffer(stride=4, count=4096)
uniform RWStructuredBuffer<DType>.Handle weightGradBuffer;

// Bias gradient (written by bias reduce)
// TEST_INPUT: set biasGradBuffer = ubuffer(stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasGradBuffer;

// TEST_INPUT:ubuffer(stride=4, count=3):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = 1024 * sizeof(uint4);
}

void clearWeightBuffer<int PaddedSize>(uint tid)
{
    static const int TotalElements = PaddedSize * PaddedSize;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    [ForceUnroll]
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            weightBuffer[index] = DType(0);
    }
}

void clearGradBuffers<int PaddedSize>(uint tid)
{
    static const int TotalElements = PaddedSize * PaddedSize;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    [ForceUnroll]
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            weightGradBuffer[index] = DType(0);
    }

    static const int BiasIters = (PaddedSize + BatchSize - 1) / BatchSize;
    [ForceUnroll]
    for (int iter = 0; iter < BiasIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < PaddedSize)
            biasGradBuffer[index] = DType(0);
    }
}

// Write identity matrix in tiled layout.
// On CUDA, also converts from row-major tiled to native fragment layout.
void identityWeightTiled<int Size, int PaddedSize>(uint tid)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedSize / TileSize;

    clearWeightBuffer<PaddedSize>(tid);
    AllMemoryBarrierWithGroupSync();

    static const int NumDiagIters = (Size + BatchSize - 1) / BatchSize;
    [ForceUnroll]
    for (int iter = 0; iter < NumDiagIters; iter++)
    {
        int row = iter * BatchSize + tid;
        if (row >= Size)
            break;
        int tileRow = row / TileSize;
        int tileCol = row / TileSize;
        int localRow = row % TileSize;
        int localCol = row % TileSize;
        int tileIndex = tileRow * NTileCols + tileCol;
        int offset = tileIndex * TileSize * TileSize + localRow * TileSize + localCol;
        weightBuffer[offset] = DType(1);
    }
    AllMemoryBarrierWithGroupSync();

    convertTiledToNativeB<DType, PaddedSize, PaddedSize, BatchSize>(tid, weightBuffer);
    AllMemoryBarrierWithGroupSync();
}

void test<int Size>(uint tid, int resIndex)
{
    static const int PaddedSize = PadToTile<Size>.Value;

    DType inputData[Size];
    DType dOutData[Size];
    for (int i = 0; i < Size; i++)
    {
        inputData[i] = DType(1);
        dOutData[i] = DType(tid + 1);
    }

    clearGradBuffers<PaddedSize>(tid);
    AllMemoryBarrierWithGroupSync();

    // W = identity → dInput = W^T * dOut = dOut
    let dInput = neural_unittest.testTiledBackward<DType, Size, Size, TestShMemSize, 1>(
        inputData, dOutData, weightBuffer, weightGradBuffer, biasGradBuffer);

    bool res = true;
    for (int i = 0; i < Size; i++)
    {
        DType expected = DType(tid + 1);
        if (!equals(dInput[i], expected))
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
    // 16x16
    {
        identityWeightTiled<16, 16>(tid);
        test<16>(tid, 0);
    }

    // 32x32
    {
        identityWeightTiled<32, 32>(tid);
        test<32>(tid, 1);
    }

    // 64x64
    {
        identityWeightTiled<64, 64>(tid);
        test<64>(tid, 2);
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
