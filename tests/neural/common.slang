#pragma warning(disable: 41017)


public bool equals<T>(T a, T b) where T : __BuiltinFloatingPointType
{
    return abs(a - b) < T(1e-2f);
}

public void initBias<T, int Size, int WorkgroupSize = 32>(uint tid, RWStructuredBuffer<T> biasBuffer)
    where T : __BuiltinFloatingPointType
{
    static const int NumIters = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * WorkgroupSize + tid;
        if (index < Size)
            biasBuffer[index] = T(index);
    }

    AllMemoryBarrierWithGroupSync();
}

public void initBias<T, int Size, int WorkgroupSize = 32>(uint tid, T* biasBuffer)
    where T : __BuiltinFloatingPointType
{
    static const int NumIters = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * WorkgroupSize + tid;
        if (index < Size)
            biasBuffer[index] = T(index);
    }

    AllMemoryBarrierWithGroupSync();
}

public bool collectResults<int BatchSize>(uint tid, bool perThreadResult)
{
    static groupshared bool s_resultMem[BatchSize / 32];

    // each warp will check the result
    bool res = WaveActiveAllTrue(perThreadResult);

    int subgroupIndex = tid / 32;
    int laneId = WaveGetLaneIndex();
    // use first lane of each warp to write the result to the shared memory so that
    // we can reduce the result further
    if (laneId == 0)
    {
        // Note max subgroupIndex is "BatchSize/32 - 1"
        s_resultMem[subgroupIndex] = res;
    }

    AllMemoryBarrierWithGroupSync();

    int warpCount = BatchSize / 32;
    bool finalResult = true;
    // We will use the "warpCount" threads in the first warp to reduce the result.
    if (tid < warpCount)
    {
        finalResult = s_resultMem[tid];
    }

    finalResult = WaveActiveAllTrue(finalResult);

    return finalResult;
}

// Pad a size up to the next multiple of 16 (cooperative matrix tile size).
public struct PadToTile<int Size>
{
    public static const int Value = ((Size + 15) / 16) * 16;
}

public void setBufferOneValue<int Size, int WorkgroupSize, T>(uint tid, RWStructuredBuffer<T> buffer, T value)
    where T : __BuiltinFloatingPointType
{
    int numIter = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int i = 0; i < numIter; i++)
    {
        int index = i * WorkgroupSize + tid;
        if (index >= Size)
            break;

        buffer[index] = value;
    }

    AllMemoryBarrierWithGroupSync();
}

// Map (row, col) inside a 16×16 tile to CUDA MatB native fragment index.
// Layout: lane = gid*4+tidLocal, each lane holds 8 elements.
//   gid = col % 8,  tidLocal = (row % 8) / 2
//   Sub-tiles split at row 8 and col 8.
int rowMajorToNativeMatB(int row, int col)
{
    int rowGroup   = row < 8 ? 0 : 1;
    int rowInGroup = row - 8 * rowGroup;
    int tidLocal   = rowInGroup / 2;
    int pair       = rowInGroup % 2;
    int sub        = col < 8 ? 0 : 1;
    int gid        = col - 8 * sub;
    int lane       = gid * 4 + tidLocal;
    int localPos   = sub * 4 + rowGroup * 2 + pair;
    return lane * 8 + localPos;
}

// Offset for element (row, col) in the weight buffer's tiled layout.
// On CUDA: native MatB fragment layout (for loadWeightRaw).
// On Vulkan: row-major within each 16×16 tile (for CoopMat.Load<RowMajor>).
// Single-tile version (row, col both < 16).
// row = output dimension (M), col = input dimension (K).
// CUDA weight is MatB<K, M> — native layout indexed as (K_row, M_col).
// Vulkan weight is MatA<M, K> — row-major indexed as (M_row, K_col).
public int tiledWeightOffset(int row, int col)
{
    __target_switch
    {
    case cuda:  return rowMajorToNativeMatB(col, row);
    case spirv: return row * 16 + col;
    }
}

// Offset for element (row, col) in the dWeight buffer's tiled layout.
// On CUDA: native MatC fragment layout (from storeRaw + atomicAdd).
// On Vulkan: row-major within each 16×16 tile (from storeTileRowToGlobal).
// Single-tile version (row, col both < 16).
public int tiledDWeightOffset(int row, int col)
{
    __target_switch
    {
    case cuda:  return rowMajorToNativeMatC(row, col);
    case spirv: return row * 16 + col;
    }
}

// Convert a row-major 16x16 tile to MatB native register layout.
// MatB packs vertically: same column (gid), adjacent rows (tid*2, tid*2+1).
public void convertTileRowMajorToNativeB<T>(
    uint tid, int srcTileOffset, int dstOffset,
    RWStructuredBuffer<T>.Handle buffer)
    where T : __BuiltinFloatingPointType
{
    static const int S = 16;
    int gid = int(tid) >> 2;
    int tidLocal = int(tid) & 3;

    T b[8];
    b[0] = buffer[srcTileOffset + (tidLocal*2)   * S + gid];
    b[1] = buffer[srcTileOffset + (tidLocal*2+1) * S + gid];
    b[2] = buffer[srcTileOffset + (tidLocal*2+8) * S + gid];
    b[3] = buffer[srcTileOffset + (tidLocal*2+9) * S + gid];
    b[4] = buffer[srcTileOffset + (tidLocal*2)   * S + gid + 8];
    b[5] = buffer[srcTileOffset + (tidLocal*2+1) * S + gid + 8];
    b[6] = buffer[srcTileOffset + (tidLocal*2+8) * S + gid + 8];
    b[7] = buffer[srcTileOffset + (tidLocal*2+9) * S + gid + 8];

    int nativeBase = dstOffset + int(tid) * 8;
    [ForceUnroll]
    for (int i = 0; i < 8; i++)
        buffer[nativeBase + i] = b[i];
}

// Convert all tiles in a PaddedRows x PaddedCols tiled matrix from
// row-major tiled layout to native MatB register layout (CUDA only).
// Uses TempOffset = PaddedRows*PaddedCols as scratch space in the same buffer.
public void convertTiledToNativeB<T, int PaddedRows, int PaddedCols, int WorkgroupSize>(
    uint tid, RWStructuredBuffer<T>.Handle buffer)
    where T : __BuiltinFloatingPointType
{
    __target_switch
    {
    case cuda:
    {
        static const int TileSize = 16;
        static const int NTileRows = PaddedRows / TileSize;
        static const int NTileCols = PaddedCols / TileSize;
        static const int TotalTiles = NTileRows * NTileCols;
        static const int TileElements = TileSize * TileSize;
        static const int TotalElements = PaddedRows * PaddedCols;
        static const int TempOffset = TotalElements;

        if (tid < 32)
        {
            for (int t = 0; t < TotalTiles; t++)
                convertTileRowMajorToNativeB<T>(tid, t * TileElements, TempOffset + t * TileElements, buffer);
        }
        AllMemoryBarrierWithGroupSync();

        static const int CopyIters = (TotalElements + WorkgroupSize - 1) / WorkgroupSize;
        [ForceUnroll]
        for (int iter = 0; iter < CopyIters; iter++)
        {
            int index = iter * WorkgroupSize + tid;
            if (index < TotalElements)
                buffer[index] = buffer[TempOffset + index];
        }
        AllMemoryBarrierWithGroupSync();
    }
    case spirv:
        break;
    }
}

// Map (row, col) inside a 16×16 tile to the CUDA MatC native fragment index.
// Layout for CoopMat<half, 16, 16, Accumulator> stored via storeRaw:
//   Lane L (g=L/4, c=L%4) holds 4 regs × 2 halves = 8 elements:
//     reg 0: (g, 2c), (g, 2c+1)          sub-tile 0
//     reg 1: (g+8, 2c), (g+8, 2c+1)      sub-tile 0
//     reg 2: (g, 2c+8), (g, 2c+9)        sub-tile 1
//     reg 3: (g+8, 2c+8), (g+8, 2c+9)    sub-tile 1
int rowMajorToNativeMatC(int row, int col)
{
    int g      = row < 8 ? row : row - 8;
    int regSub = row < 8 ? 0 : 1;
    int sub    = col < 8 ? 0 : 1;
    int c      = (col - 8 * sub) / 2;
    int hp     = col % 2;
    int lane   = g * 4 + c;
    int loc    = sub * 4 + regSub * 2 + hp;
    return lane * 8 + loc;
}

// Convert CUDA outer product output from native layout (col-major tiles,
// native MatC fragment order within tiles) to row-major tiled layout.
// On Vulkan the output is already row-major, so this is a no-op.
//
// Each thread reads its share of elements into registers, barrier, then
// writes back to the remapped positions.  No scratch buffer needed.
public void convertNativeToRowMajorTiled<T, int PaddedRows, int PaddedCols, int WorkgroupSize>(
    uint tid, RWStructuredBuffer<T>.Handle buffer)
    where T : __BuiltinFloatingPointType
{
    __target_switch
    {
    case cuda:
    {
        static const int TileSize = 16;
        static const int NTileRows = PaddedRows / TileSize;
        static const int NTileCols = PaddedCols / TileSize;
        static const int TileElements = TileSize * TileSize;
        static const int TotalElements = PaddedRows * PaddedCols;
        static const int ElemsPerThread = (TotalElements + WorkgroupSize - 1) / WorkgroupSize;

        T regs[ElemsPerThread];
        int dstIdx[ElemsPerThread];

        [ForceUnroll]
        for (int i = 0; i < ElemsPerThread; i++)
        {
            int srcLinear = i * WorkgroupSize + int(tid);
            if (srcLinear < TotalElements)
            {
                regs[i] = buffer[srcLinear];

                int tileSeq  = srcLinear / TileElements;
                int localNat = srcLinear % TileElements;
                int tileRow  = tileSeq % NTileRows;
                int tileCol  = tileSeq / NTileRows;

                int lane    = localNat / 8;
                int loc     = localNat % 8;
                int g       = lane / 4;
                int c       = lane % 4;
                int sub     = loc / 4;
                int regSub  = (loc % 4) / 2;
                int hp      = loc % 2;
                int row     = g + regSub * 8;
                int col     = 2 * c + 8 * sub + hp;

                dstIdx[i] = (tileRow * NTileCols + tileCol) * TileElements + row * TileSize + col;
            }
            else
            {
                dstIdx[i] = -1;
            }
        }

        AllMemoryBarrierWithGroupSync();

        [ForceUnroll]
        for (int i = 0; i < ElemsPerThread; i++)
        {
            if (dstIdx[i] >= 0)
                buffer[dstIdx[i]] = regs[i];
        }

        AllMemoryBarrierWithGroupSync();
    }
    case spirv:
        break;
    }
}

// Fill weight W[r][c] = scaleFactor(r) * (c+1) in tiled layout for transpose MMA tests.
// Rows = OutputSize (M), Cols = InputSize (K).
// CUDA: stores K×M with native MatB conversion. Vulkan: stores M×K for loadWeightTiled.
public void fillTransposeWeightTiled<T, int Rows, int Cols, int WorkgroupSize, bool Clear>(
    uint tid, RWStructuredBuffer<T>.Handle buffer)
    where T : __BuiltinFloatingPointType
{
    static const int TileSize = 16;
    static const int PaddedRows = PadToTile<Rows>.Value;
    static const int PaddedCols = PadToTile<Cols>.Value;
    static const int TotalElements = PaddedRows * PaddedCols;
    static const int NumIters = (TotalElements + WorkgroupSize - 1) / WorkgroupSize;

    if (Clear)
    {
        for (int iter = 0; iter < NumIters; iter++)
        {
            int idx = iter * WorkgroupSize + tid;
            if (idx < TotalElements)
                buffer[idx] = T(0);
        }
        return;
    }

    for (int iter = 0; iter < NumIters; iter++)
    {
        int idx = iter * WorkgroupSize + tid;
        if (idx < TotalElements)
            buffer[idx] = T(0);
    }
    AllMemoryBarrierWithGroupSync();

    __target_switch
    {
    case cuda:
    {
        static const int NTileCols = PaddedRows / TileSize;
        static const int NumRowIters = (Cols + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumRowIters; iter++)
        {
            int row = iter * WorkgroupSize + tid;
            if (row >= Cols) break;
            int tileRow = row / TileSize;
            int localRow = row % TileSize;
            for (int col = 0; col < Rows; col++)
            {
                int scaleFactor = (col % 3 == 0) ? -1 : ((col % 3 == 1) ? 0 : 1);
                int tileCol = col / TileSize;
                int localCol = col % TileSize;
                int offset = (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
                buffer[offset] = T(scaleFactor * (row + 1));
            }
        }
        AllMemoryBarrierWithGroupSync();
        convertTiledToNativeB<T, PaddedCols, PaddedRows, WorkgroupSize>(tid, buffer);
    }
    case spirv:
    {
        static const int NTileCols = PaddedCols / TileSize;
        static const int NumRowIters = (Rows + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumRowIters; iter++)
        {
            int row = iter * WorkgroupSize + tid;
            if (row >= Rows) break;
            int scaleFactor = (row % 3 == 0) ? -1 : ((row % 3 == 1) ? 0 : 1);
            int tileRow = row / TileSize;
            int localRow = row % TileSize;
            for (int col = 0; col < Cols; col++)
            {
                int tileCol = col / TileSize;
                int localCol = col % TileSize;
                int offset = (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
                buffer[offset] = T(scaleFactor * (col + 1));
            }
        }
    }
    }
}

// Fill weight matrix with a constant value in tiled layout for forward MMA tests.
// InputSize = K, OutputSize = M.
// CUDA: stores as B(K, M) then converts to native fragment layout.
// VK:   stores as A(M, K) in row-major tiled layout (what loadWeightTiled expects).
public void fillForwardWeightTiled<T, int InputSize, int OutputSize, int WorkgroupSize>(
    uint tid, RWStructuredBuffer<T>.Handle buffer, T fillValue)
    where T : __BuiltinFloatingPointType
{
    static const int TileSize = 16;
    static const int PaddedK = PadToTile<InputSize>.Value;
    static const int PaddedM = PadToTile<OutputSize>.Value;

    __target_switch
    {
    case cuda:
    {
        static const int TotalElements = PaddedM * PaddedK;
        static const int NumClearIters = (TotalElements + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumClearIters; iter++)
        {
            int idx = iter * WorkgroupSize + tid;
            if (idx < TotalElements)
                buffer[idx] = T(0);
        }
        AllMemoryBarrierWithGroupSync();

        static const int NTileCols = PaddedM / TileSize;
        static const int NumRowIters = (InputSize + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumRowIters; iter++)
        {
            int row = iter * WorkgroupSize + tid;
            if (row >= InputSize) break;
            int tileRow = row / TileSize;
            int localRow = row % TileSize;
            for (int col = 0; col < OutputSize; col++)
            {
                int tileCol = col / TileSize;
                int localCol = col % TileSize;
                int offset = (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
                buffer[offset] = fillValue;
            }
        }
        AllMemoryBarrierWithGroupSync();
        convertTiledToNativeB<T, PaddedK, PaddedM, WorkgroupSize>(tid, buffer);
    }
    default:
    {
        static const int TotalElements = PaddedK * PaddedM;
        static const int NumClearIters = (TotalElements + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumClearIters; iter++)
        {
            int idx = iter * WorkgroupSize + tid;
            if (idx < TotalElements)
                buffer[idx] = T(0);
        }
        AllMemoryBarrierWithGroupSync();

        static const int NTileCols = PaddedK / TileSize;
        static const int NumRowIters = (OutputSize + WorkgroupSize - 1) / WorkgroupSize;
        for (int iter = 0; iter < NumRowIters; iter++)
        {
            int row = iter * WorkgroupSize + tid;
            if (row >= OutputSize) break;
            int tileRow = row / TileSize;
            int localRow = row % TileSize;
            for (int col = 0; col < InputSize; col++)
            {
                int tileCol = col / TileSize;
                int localCol = col % TileSize;
                int offset = (tileRow * NTileCols + tileCol) * (TileSize * TileSize) + localRow * TileSize + localCol;
                buffer[offset] = fillValue;
            }
        }
        AllMemoryBarrierWithGroupSync();
    }
    }
}
