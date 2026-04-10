// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=0 -xslang -DTEST_BIAS=1
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_HALF=1 -xslang -DTEST_BIAS=1

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

// TEST_INPUT: set inputBuffer = ubuffer(stride=4, count=8192)
uniform RWStructuredBuffer<DType>.Handle inputBuffer;

// TEST_INPUT: set biasBuffer = ubuffer(stride=4, count=64)
uniform RWStructuredBuffer<DType>.Handle biasBuffer;

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
            inputBuffer[index] = DType(0);
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
// Convert a row-major 16x16 tile to MatrixA native register layout.
// The forward path loads weight as MatA. Native layout for lane l (gid=l>>2, tid=l&3):
//   reg[0] = pack(A[gid][tid*2],     A[gid][tid*2+1])       rows 0-7,  cols 0-7
//   reg[1] = pack(A[gid+8][tid*2],   A[gid+8][tid*2+1])     rows 8-15, cols 0-7
//   reg[2] = pack(A[gid][tid*2+8],   A[gid][tid*2+9])       rows 0-7,  cols 8-15
//   reg[3] = pack(A[gid+8][tid*2+8], A[gid+8][tid*2+9])     rows 8-15, cols 8-15
// MatA packs horizontally: same row, adjacent columns.
// Convert a row-major 16x16 tile to MatB native register layout for loadTileRawFromPointer.
// MatB packs vertically: same column (gid), adjacent rows (tid*2, tid*2+1).
// For lane l (gid=l>>2, tid=l&3), the 8 elements in native order are:
//   B[tid*2][gid], B[tid*2+1][gid],       B[tid*2+8][gid], B[tid*2+9][gid],
//   B[tid*2][gid+8], B[tid*2+1][gid+8],   B[tid*2+8][gid+8], B[tid*2+9][gid+8]
void convertTileRowMajorToNativeB(
    uint tid,
    int srcTileOffset,
    int dstOffset,
    RWStructuredBuffer<DType>.Handle buffer)
{
    static const int T = 16;

    int gid = int(tid) >> 2;
    int tidLocal = int(tid) & 3;

    // Read from row-major: element B[row][col] at srcTileOffset + row*16 + col
    DType b[8];
    b[0] = buffer[srcTileOffset + (tidLocal*2)   * T + gid];
    b[1] = buffer[srcTileOffset + (tidLocal*2+1) * T + gid];
    b[2] = buffer[srcTileOffset + (tidLocal*2+8) * T + gid];
    b[3] = buffer[srcTileOffset + (tidLocal*2+9) * T + gid];
    b[4] = buffer[srcTileOffset + (tidLocal*2)   * T + gid + 8];
    b[5] = buffer[srcTileOffset + (tidLocal*2+1) * T + gid + 8];
    b[6] = buffer[srcTileOffset + (tidLocal*2+8) * T + gid + 8];
    b[7] = buffer[srcTileOffset + (tidLocal*2+9) * T + gid + 8];

    int nativeBase = dstOffset + int(tid) * 8;
    buffer[nativeBase + 0] = b[0];
    buffer[nativeBase + 1] = b[1];
    buffer[nativeBase + 2] = b[2];
    buffer[nativeBase + 3] = b[3];
    buffer[nativeBase + 4] = b[4];
    buffer[nativeBase + 5] = b[5];
    buffer[nativeBase + 6] = b[6];
    buffer[nativeBase + 7] = b[7];
}

void identityWeightMatrixTiled<int InputSize, int PaddedSize>(uint tid, bool clear = false)
{
    static const int TileSize = 16;
    static const int NTileCols = PaddedSize / TileSize;
    static const int NTileRows = PaddedSize / TileSize;

    if (clear)
    {
        clearWeightBuffer<PaddedSize>(tid);
        return;
    }

    // First clear, then write identity in row-major tiled layout.
    clearWeightBuffer<PaddedSize>(tid);
    AllMemoryBarrierWithGroupSync();

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
        inputBuffer[offset] = DType(1);
    }

    AllMemoryBarrierWithGroupSync();

    // On CUDA, convert row-major tiled → native MatrixB layout (in-place).
    // On Vulkan, the row-major tiled layout is used directly by CoopMat.Load.
    __target_switch
    {
    case cuda:
    {
        // Native layout: 256 DType elements per tile (32 lanes × 8 elements).
        static const int TotalTiles = NTileRows * NTileCols;
        static const int TileElements = TileSize * TileSize; // 256
        static const int TotalElements = PaddedSize * PaddedSize;

        // Temp region starts after the row-major data (in DType units)
        static const int TempOffset = TotalElements;

        // Convert each tile: read row-major, write native, both in DType units
        for (int t = 0; t < TotalTiles; t++)
        {
            convertTileRowMajorToNativeB(tid, t * TileElements, TempOffset + t * TileElements, inputBuffer);
        }
        AllMemoryBarrierWithGroupSync();

        // Copy native data from temp back to buffer start
        static const int CopyIters = (TotalElements + BatchSize - 1) / BatchSize;
        [ForceUnroll]
        for (int iter = 0; iter < CopyIters; iter++)
        {
            int index = iter * BatchSize + tid;
            if (index < TotalElements)
                inputBuffer[index] = inputBuffer[TempOffset + index];
        }
        AllMemoryBarrierWithGroupSync();
    }
    case spirv:
        break;
    }
}

void test<int InputSize, int OutputSize>(uint tid, int resIndex)
{
    static const int PaddedInputSize = PadToTile<InputSize>.Value;
    static const int PaddedOutputSize = PadToTile<OutputSize>.Value;

    DType inputData[InputSize];
    for (int i = 0; i < InputSize; i++)
        inputData[i] = DType(tid + 1);

#if TEST_BIAS
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, true, 1, false>(
        inputData, inputBuffer, Optional<RWStructuredBuffer<DType>.Handle>(biasBuffer));
#else
    let output = neural_unittest.testTiledMatVecMul<DType, InputSize, OutputSize, InputSize, PaddedInputSize, PaddedOutputSize, TestShMemSize, false, 1, false>(
        inputData, inputBuffer, none);
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

    res = WaveActiveAllTrue(res);
    if (tid == 0)
        outputBuffer[resIndex] = res ? 1 : 0;
}

[numthreads(BatchSize, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_DispatchThreadID)
{
    initBias<DType, 64, BatchSize>(tid, biasBuffer);

    // 4x4 (padded to 16x16)
    {
        identityWeightMatrixTiled<4, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<4, 4>(tid, 0);
        identityWeightMatrixTiled<4, 16>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    // 8x8 (padded to 16x16)
    {
        identityWeightMatrixTiled<8, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<8, 8>(tid, 1);
        identityWeightMatrixTiled<8, 16>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    // 16x16 (already aligned)
    {
        identityWeightMatrixTiled<16, 16>(tid);
        AllMemoryBarrierWithGroupSync();
        test<16, 16>(tid, 2);
        identityWeightMatrixTiled<16, 16>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    // 32x32 (already aligned)
    {
        identityWeightMatrixTiled<32, 32>(tid);
        AllMemoryBarrierWithGroupSync();
        test<32, 32>(tid, 3);
        identityWeightMatrixTiled<32, 32>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    // 64x64 (already aligned)
    {
        identityWeightMatrixTiled<64, 64>(tid);
        AllMemoryBarrierWithGroupSync();
        test<64, 64>(tid, 4);
        identityWeightMatrixTiled<64, 64>(tid, true);
        AllMemoryBarrierWithGroupSync();
    }

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
//
