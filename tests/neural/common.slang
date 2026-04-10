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
