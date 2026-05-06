// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1


#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

struct TestBuffers
{
    RWStructuredBuffer<DType>.Handle inputBuffer;
    RWStructuredBuffer<DType>.Handle biasBuffer;
}

// TEST_INPUT: set buffers.inputBuffer = ubuffer(stride=4, count=8192)
// TEST_INPUT: set buffers.biasBuffer = ubuffer(stride=4, count=64)
ParameterBlock<TestBuffers> buffers;

// TEST_INPUT:ubuffer(stride=4, count=5):out, name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const int BatchSize = 32;
static const int SubgroupSize = 32;

public struct TestShMemSize : ISharedMemorySize {
    public static const uint Bytes = (256 + 256) * sizeof(uint4);
}
static const uint OffsetA = 0;
static const uint OffsetB = 256;

// Clear the weight buffer region used by a PaddedSize x PaddedSize tiled matrix.
void clearWeightBuffer<int PaddedSize>(uint tid)
{
    static const int TotalElements = PaddedSize * PaddedSize;
    static const int NumIters = (TotalElements + BatchSize - 1) / BatchSize;

    [ForceUnroll]
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * BatchSize + tid;
        if (index < TotalElements)
            buffers.inputBuffer[index] = DType(0);
    }
}

// Write an identity matrix in tiled layout.
// The weight matrix is PaddedSize x PaddedSize (both multiples of 16),
// but only the top-left InputSize x InputSize has the identity.
//
// Tiled layout addressing for element (row, col):
//   tileRow   = row / 16
//   tileCol   = col / 16
//   NTileCols = PaddedSize / 16
//   tileIndex = tileRow * NTileCols + tileCol
//   offset    = tileIndex * 256 + (row % 16) * 16 + (col % 16)
void identityWeightMatrixTiled<int InputSize, int PaddedSize>(uint tid, bool clear = false)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedSize / TileSize;

    if (clear)
    {
        clearWeightBuffer<PaddedSize>(tid);
        return;
    }

    clearWeightBuffer<PaddedSize>(tid);
    AllMemoryBarrierWithGroupSync();

    __target_switch
    {
    case metal:
        {
            fillForwardWeightTiled<DType, InputSize, InputSize, BatchSize>(
                tid, buffers.inputBuffer, DType(1.0f / float(InputSize)));
        }
    default:
        {
            static const int NumDiagIters = (InputSize + BatchSize - 1) / BatchSize;
            [ForceUnroll]
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
                buffers.inputBuffer[offset] = DType(1);
            }

            AllMemoryBarrierWithGroupSync();
            convertTiledToNativeB<DType, PaddedSize, PaddedSize, BatchSize>(tid, buffers.inputBuffer);
        }
    }
}

bool test<int InputSize, int OutputSize>(uint tid)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    typealias InputVec = WaveTangledVector<DType, TestShMemSize, PaddedInputSize, SubgroupSize, 1>;
    InputVec inputData = InputVec(DType(0));
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 1, false>(
        inputData, buffers.inputBuffer, Optional<RWStructuredBuffer<DType>.Handle>(buffers.biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, PaddedInputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, false>(
        inputData, buffers.inputBuffer, none);
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

    return WaveActiveAllTrue(res);
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex, uint3 gid : SV_GroupID)
{
    if (gid.x != 0)
        return;

    initBias<DType, 64, BatchSize>(tid, buffers.biasBuffer);

    // 4x4 (padded to 16x16)
    {
        identityWeightMatrixTiled<4, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<4, 4>(tid);
        if (tid == 0)
            outputBuffer[0] = result ? 1 : 0;
        AllMemoryBarrierWithGroupSync();
    }

    // 8x8 (padded to 16x16)
    {
        identityWeightMatrixTiled<8, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<8, 8>(tid);
        if (tid == 0)
            outputBuffer[1] = result ? 1 : 0;
        AllMemoryBarrierWithGroupSync();
    }

    // 16x16 (already aligned)
    {
        identityWeightMatrixTiled<16, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<16, 16>(tid);
        if (tid == 0)
            outputBuffer[2] = result ? 1 : 0;
        AllMemoryBarrierWithGroupSync();
    }

    // 32x32 (already aligned)
    {
        identityWeightMatrixTiled<32, 32>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<32, 32>(tid);
        if (tid == 0)
            outputBuffer[3] = result ? 1 : 0;
        AllMemoryBarrierWithGroupSync();
    }

    // 64x64 (already aligned)
    {
        identityWeightMatrixTiled<64, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        bool result = test<64, 64>(tid);
        if (tid == 0)
            outputBuffer[4] = result ? 1 : 0;
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
    // BUFFER-NEXT: {{^}}1{{$}}
}
//
