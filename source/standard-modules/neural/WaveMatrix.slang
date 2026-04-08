implementing neural;


// WaveMatrix: A tiled matrix type over 16x16 cooperative matrix fragments.
//
// Maps directly to Tin2's MatrixBase<T, N_ROWS, N_COLS, ROW_MAJOR>:
//   WaveMatrix<half, MatrixA, R, C> ≡ Tin2's MatrixA<half, R, C> (ROW_MAJOR=true)
//   WaveMatrix<half, MatrixB, R, C> ≡ Tin2's MatrixB<half, R, C> (ROW_MAJOR=false)
//
// Internally stores an array of 16x16 CoopMat tiles arranged in column-major
// tile order (matching Tin2's mma_rc_to_idx = col * MMARows + row).
//
// Template parameters:
//   T    - element type (half for MatA/MatB, half or float for MatC/Accumulator)
//   Use  - linalg.CoopMatMatrixUse (MatrixA, MatrixB, or MatrixAccumulator)
//   Rows - total row count (must be multiple of 16)
//   Cols - total column count (must be multiple of 16)

internal typealias MatrixA<int Rows, int Cols> = WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixA, Rows, Cols>;
internal typealias MatrixB<int Rows, int Cols> = WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixB, Rows, Cols>;
internal typealias MatrixC<T: __BuiltinFloatingPointType, int Rows, int Cols> = WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixAccumulator, Rows, Cols>;

[require(cuda)]
internal struct WaveMatrix<T, linalg.CoopMatMatrixUse Use, int Rows, int Cols>
    where T : __BuiltinFloatingPointType
{
    static const int TILE = 16;
    static const int TileRows = (Rows + TILE - 1) / TILE;
    static const int TileCols = (Cols + TILE - 1) / TILE;
    internal static const int TileCount = TileRows * TileCols;

    typealias Tile = linalg.CoopMat<T, MemoryScope.Subgroup, TILE, TILE, Use>;

    static const int ElementsPerLane = (TILE * TILE) / 32;
    static const int RegsPerLane = (ElementsPerLane * sizeof(T) + 3) / 4;
    static const int TileStride = (RegsPerLane * 32 * 4 + sizeof(uint4) - 1) / sizeof(uint4);

    internal Tile tiles[TileCount];

    // Column-major tile index (matches Tin2's mma_rc_to_idx = col * MMARows + row)
    [ForceInline]
    static int tileIdx(int tileRow, int tileCol)
    {
        return tileCol * TileRows + tileRow;
    }

    // =========================================================================
    // changeMajor: apply movmatrix.trans per tile.
    //
    // Converts each sub-matrix in place. The physical register bits are
    // transposed within each 8x8 sub-register via the movmatrix PTX instruction.
    //
    // Note: this does NOT remap the tile grid — it only transforms each tile
    // individually. For a full Tin2-style change_major_axis, the caller must
    // also handle tile grid reordering if needed.
    // =========================================================================

    [ForceInline]
    [mutating]
    void changeMajor()
    {
        [ForceUnroll]
        for (int i = 0; i < TileCount; i++)
            tiles[i].ChangeMajor();
    }

    // =========================================================================
    // transposeSubMatrix: sub-tile transpose (swap reg[1] <-> reg[2])
    // within each 16x16 tile, without swapping tile positions.
    //
    // Equivalent to the inner part of Tin2's .transpose() that operates
    // on each MMAMatrix independently.
    // =========================================================================

    [ForceInline]
    [mutating]
    void transposeSubMatrix()
    {
        [ForceUnroll]
        for (int idx = 0; idx < TileCount; idx++)
        {
            uint r1 = tiles[idx].fragmentRead(1);
            uint r2 = tiles[idx].fragmentRead(2);
            tiles[idx].fragmentWrite(1, r2);
            tiles[idx].fragmentWrite(2, r1);
        }
    }

    // =========================================================================
    // add: element-wise addition of two matrices of the same shape and Use.
    //
    // Equivalent to Tin2's operator+(MatrixBase, MatrixBase).
    // =========================================================================

    [ForceInline]
    WaveMatrix<T, Use, Rows, Cols> add(WaveMatrix<T, Use, Rows, Cols> other)
    {
        WaveMatrix<T, Use, Rows, Cols> result;
        [ForceUnroll]
        for (int i = 0; i < TileCount; i++)
            result.tiles[i] = tiles[i] + other.tiles[i];
        return result;
    }

    // =========================================================================
    // fromArrayPacked: convert per-thread packed uint array into MatA tiles.
    //
    // Each thread holds PackedSize uint32 values (each packing 2 half values).
    // The function distributes them across tiles using warp shuffles.
    // Only valid for MatrixA use.
    //
    // Equivalent to Tin2's Vector::from_array (optimized shuffle path).
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    [mutating]
    internal void fromArrayPacked<int PackedSize>(uint packedData[PackedSize])
    {
        static_assert(Use == linalg.CoopMatMatrixUse.MatrixA, "fromArrayPacked only supports MatrixA");

        uint laneid = WaveGetLaneIndex();

        static const int TotalColsPacked = PackedSize;
        static const int TileColsPacked = TILE / 2;
        static const uint RegColsPacked = 4;
        static const uint RegRowsPacked = 8;
        static const uint RegRows = 2;
        static const uint MMARows = 2;

        [ForceUnroll]
        for (uint col_packed = 0; col_packed < uint(TotalColsPacked); col_packed += RegColsPacked)
        {
            uint regsi[4];

            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                regsi[i] = packedData[int(col_packed + i)];

                [ForceUnroll]
                for (uint j = 1; j < RegColsPacked; j++)
                {
                    uint alt_src = col_packed + (i + j) % RegColsPacked;
                    uint val_packed = packedData[int(alt_src)];
                    if (laneid / RegRowsPacked == j)
                        regsi[i] = val_packed;
                }
            }

            WaveMask fullMask = WaveMask(0xFFFFFFFF);
            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                uint shfl_idx = ((laneid + (RegColsPacked - i)) % RegColsPacked) * RegRowsPacked + laneid / RegColsPacked;
                regsi[i] = WaveMaskReadLaneAt(fullMask, regsi[i], int(shfl_idx));
            }

            uint mma_col = col_packed / uint(TileColsPacked);
            uint r_col = (col_packed % uint(TileColsPacked)) / RegColsPacked;

            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                uint r_row = i % RegRows;
                uint mma_row = i / RegRows;

                int reg_idx = int(r_col * RegRows + r_row);
                int frag_idx = int(mma_col * MMARows + mma_row);

                uint value = regsi[0];
                [ForceUnroll]
                for (uint j = 1; j < RegColsPacked; j++)
                {
                    if (laneid % RegColsPacked == (i + j) % RegColsPacked)
                        value = regsi[j];
                }

                tiles[frag_idx].fragmentWrite(reg_idx, value);
            }
        }
    }

    // =========================================================================
    // toArrayPacked: extract per-thread packed uint array from MatA tiles.
    //
    // Inverse of fromArrayPacked. Each thread gets PackedSize uint32 values
    // (each packing 2 half values) via warp shuffles.
    // Only valid for MatrixA use.
    //
    // Equivalent to Tin2's Vector::to_array (optimized shuffle path).
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal void toArrayPacked<int PackedSize>(inout uint packedOut[PackedSize])
    {
        static_assert(Use == linalg.CoopMatMatrixUse.MatrixA
                   || Use == linalg.CoopMatMatrixUse.MatrixAccumulator,
            "toArrayPacked only supports MatrixA or MatrixAccumulator");

        uint laneid = WaveGetLaneIndex();

        // For half: 2 elements per uint32, TileColsPacked = 8
        // For float: 1 element per uint32, TileColsPacked = 16
        static const int ElemsPerReg = sizeof(uint) / sizeof(T);
        static const int TotalColsPacked = PackedSize;
        static const int TileColsPacked = TILE / ElemsPerReg;
        static const uint RegColsPacked = 4;
        static const uint RegRowsPacked = 8;
        static const uint RegRows = 2;
        static const uint MMARows = 2;

        [ForceUnroll]
        for (uint col_packed = 0; col_packed < uint(TotalColsPacked); col_packed += RegColsPacked)
        {
            uint mma_col = col_packed / uint(TileColsPacked);
            uint r_col = (col_packed % uint(TileColsPacked)) / RegColsPacked;

            uint regsi[4];

            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                int base_reg_idx = int(r_col * RegRows);
                int base_frag_idx = int(mma_col * MMARows);
                regsi[i] = tiles[base_frag_idx].fragmentRead(base_reg_idx);

                [ForceUnroll]
                for (uint j = 1; j < RegColsPacked; j++)
                {
                    uint r_row = j % RegRows;
                    uint mma_row = j / RegRows;

                    int reg_idx = int(r_col * RegRows + r_row);
                    int frag_idx = int(mma_col * MMARows + mma_row);

                    if (laneid % RegColsPacked == (i + j) % RegColsPacked)
                        regsi[i] = tiles[frag_idx].fragmentRead(reg_idx);
                }
            }

            WaveMask fullMask = WaveMask(0xFFFFFFFF);
            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                uint shfl_idx = (laneid % RegRowsPacked) * RegColsPacked + (laneid / RegRowsPacked + i) % RegColsPacked;
                regsi[i] = WaveMaskReadLaneAt(fullMask, regsi[i], int(shfl_idx));
            }

            [ForceUnroll]
            for (uint i = 0; i < RegColsPacked; i++)
            {
                uint op = regsi[0];
                [ForceUnroll]
                for (uint j = 1; j < RegColsPacked; j++)
                {
                    if (laneid / RegRowsPacked == (RegColsPacked - j + i) % RegColsPacked)
                        op = regsi[j];
                }

                uint idx = col_packed + i;
                if (idx < uint(TotalColsPacked))
                    packedOut[int(idx)] = op;
            }
        }
    }

    // =========================================================================
    // loadWeightRaw: load tiles from native tiled layout in global memory.
    //
    // Weight data is stored row-major in tile order: tile(r,c) at offset
    // (r * TileCols + c) * 256 elements. Tiles are loaded via LoadRaw
    // (vectorized per-lane load, no shuffles).
    //
    // Only valid for MatrixB use.
    //
    // Equivalent to Tin2's MatrixBase::load_native() for weight matrices.
    // =========================================================================

    [ForceInline]
    [require(cuda)]
    [mutating]
    void loadTileRawFromPointer<U>(int tileIndex, U* basePtr)
    {
        uint laneid = WaveGetLaneIndex();

        // Native layout: 32 lanes × ElementsPerLane elements of type U per tile.
        // Each lane reads its elements starting at laneid * ElementsPerLane.
        static const int ElemsPerLane = (TILE * TILE) / 32;  // 8

        if (sizeof(U) == sizeof(T))
        {
            // Same size (e.g. half→half or float→float): raw uint32 copy
            let src = bit_cast<uint*>(basePtr) + laneid * RegsPerLane;

            if (RegsPerLane <= 4)
            {
                let vec = *bit_cast<uint4*>(src);
                tiles[tileIndex].fragmentWrite(0, vec.x);
                tiles[tileIndex].fragmentWrite(1, vec.y);
                tiles[tileIndex].fragmentWrite(2, vec.z);
                tiles[tileIndex].fragmentWrite(3, vec.w);
            }
            else
            {
                let vec0 = *bit_cast<uint4*>(src);
                let vec1 = *bit_cast<uint4*>(src + 4);
                tiles[tileIndex].fragmentWrite(0, vec0.x);
                tiles[tileIndex].fragmentWrite(1, vec0.y);
                tiles[tileIndex].fragmentWrite(2, vec0.z);
                tiles[tileIndex].fragmentWrite(3, vec0.w);
                tiles[tileIndex].fragmentWrite(4, vec1.x);
                tiles[tileIndex].fragmentWrite(5, vec1.y);
                tiles[tileIndex].fragmentWrite(6, vec1.z);
                tiles[tileIndex].fragmentWrite(7, vec1.w);
            }
        }
        else if (sizeof(U) > sizeof(T))
        {
            // Wider source (e.g. float→half): read U values, convert to T, pack into uint32.
            // Each lane reads ElemsPerLane values of U, converts to T,
            // packs into RegsPerLane(T) uint32 registers.
            U* src = basePtr + laneid * ElemsPerLane;

            let fvec0 = *bit_cast<float4*>(src);
            let fvec1 = *bit_cast<float4*>(src + 4);
            let tvec0 = (vector<T, 4>)fvec0;
            let tvec1 = (vector<T, 4>)fvec1;

            tiles[tileIndex].fragmentWrite(0, bit_cast<uint>(tvec0.xy));
            tiles[tileIndex].fragmentWrite(1, bit_cast<uint>(tvec0.zw));
            tiles[tileIndex].fragmentWrite(2, bit_cast<uint>(tvec1.xy));
            tiles[tileIndex].fragmentWrite(3, bit_cast<uint>(tvec1.zw));
        }
    }

    [ForceInline]
    [require(cuda)]
    [mutating]
    void loadWeightRaw<U, Address>(Address weightAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        static_assert(Use == linalg.CoopMatMatrixUse.MatrixB, "loadWeightRaw only supports MatrixB");

        static const int TileSizeInElements = TILE * TILE;

        U* basePtr;
        if (Address.BufferType is RWStructuredBuffer<U>)
        {
            let bufferPtr = __getStructuredBufferPtr(bit_cast<RWStructuredBuffer<U>>(weightAddress.getBuffer()));
            let doublePtr = bit_cast<U**>(bufferPtr);
            basePtr = doublePtr[0] + weightAddress.getBaseIndex();
        }
        else
        {
            basePtr = bit_cast<U*>(weightAddress.getBuffer()) + weightAddress.getBaseIndex();
        }

        [ForceUnroll]
        for (int r = 0; r < TileRows; r++)
        {
            [ForceUnroll]
            for (int c = 0; c < TileCols; c++)
            {
                int idx = tileIdx(r, c);
                int offset = (r * TileCols + c) * TileSizeInElements;
                loadTileRawFromPointer<U>(idx, basePtr + offset);
            }
        }
    }

    // =========================================================================
    // storeRaw / loadRaw: native fragment layout via shared memory.
    //
    // Each tile is stored as 32 lanes × RegsPerLane registers contiguously.
    // Uses uint4 vectorized shared memory access for efficiency.
    //
    // Equivalent to Tin2's store_native / load_native on shared memory.
    // =========================================================================

    [ForceInline]
    void storeRaw<ShMemSize : ISharedMemorySize>(uint baseOffset)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        uint laneId = WaveGetLaneIndex();

        [ForceUnroll]
        for (int i = 0; i < TileCount; i++)
        {
            uint offset = baseOffset + uint(i) * uint(TileStride);
            if (RegsPerLane <= 4)
            {
                uint4 v = uint4(tiles[i].fragmentRead(0), tiles[i].fragmentRead(1),
                                tiles[i].fragmentRead(2), tiles[i].fragmentRead(3));
                ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId), v);
            }
            else
            {
                uint4 v0 = uint4(tiles[i].fragmentRead(0), tiles[i].fragmentRead(1),
                                 tiles[i].fragmentRead(2), tiles[i].fragmentRead(3));
                uint4 v1 = uint4(tiles[i].fragmentRead(4), tiles[i].fragmentRead(5),
                                 tiles[i].fragmentRead(6), tiles[i].fragmentRead(7));
                ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2), v0);
                ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2 + 1), v1);
            }
        }
    }

    [ForceInline]
    [mutating]
    void loadRaw<ShMemSize : ISharedMemorySize>(uint baseOffset)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        uint laneId = WaveGetLaneIndex();

        [ForceUnroll]
        for (int i = 0; i < TileCount; i++)
        {
            uint offset = baseOffset + uint(i) * uint(TileStride);
            if (RegsPerLane <= 4)
            {
                uint4 v = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId));
                tiles[i].fragmentWrite(0, v.x);
                tiles[i].fragmentWrite(1, v.y);
                tiles[i].fragmentWrite(2, v.z);
                tiles[i].fragmentWrite(3, v.w);
            }
            else
            {
                uint4 v0 = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2));
                uint4 v1 = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2 + 1));
                tiles[i].fragmentWrite(0, v0.x);
                tiles[i].fragmentWrite(1, v0.y);
                tiles[i].fragmentWrite(2, v0.z);
                tiles[i].fragmentWrite(3, v0.w);
                tiles[i].fragmentWrite(4, v1.x);
                tiles[i].fragmentWrite(5, v1.y);
                tiles[i].fragmentWrite(6, v1.z);
                tiles[i].fragmentWrite(7, v1.w);
            }
        }
    }
}

// =========================================================================
// Conversion functions
// =========================================================================

// Convert MatrixA(R×C) → MatrixB(R×C) via copyFrom + ChangeMajor per tile,
// with tile grid remapping (column-major A → column-major B).
// Equivalent to Tin2's change_major_axis(MatrixA) → MatrixB.
[ForceInline]
[require(cuda)]
internal WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixB, Rows, Cols>
toMatrixB<int Rows, int Cols>(
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixA, Rows, Cols> input)
{
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixB, Rows, Cols> result;
    static const int TileRows = (Rows + 15) / 16;
    static const int TileCols = (Cols + 15) / 16;

    [ForceUnroll]
    for (int c = 0; c < TileCols; c++)
    {
        [ForceUnroll]
        for (int r = 0; r < TileRows; r++)
        {
            int srcIdx = c * TileRows + r;
            int dstIdx = r * TileCols + c;
            result.tiles[dstIdx].copyFrom(input.tiles[srcIdx]);
            result.tiles[dstIdx].ChangeMajor();
        }
    }
    return result;
}

// =========================================================================
// Free functions operating on WaveMatrix
// =========================================================================

// Matrix multiply-add: A(R0×C0) × B(C0×C1) + C(R0×C1) → MatA(R0×C1)
// Internally uses MatrixAccumulator for MMA, then copies result to MatA.
// Equivalent to Tin2's mad(MatrixA, MatrixB, MatrixA).
[ForceInline]
[require(cuda)]
internal void matMad<T, int R0, int C0, int C1>(
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixA, R0, C0> a,
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixB, C0, C1> b,
    inout WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixAccumulator, R0, C1> c)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
{
    static const int KTiles = (C0 + 15) / 16;
    static const int MRows = (R0 + 15) / 16;
    static const int NCols = (C1 + 15) / 16;
    static const int ResultTiles = MRows * NCols;

    [ForceUnroll]
    for (int k = 0; k < KTiles; k++)
    {
        [ForceUnroll]
        for (int mc = 0; mc < NCols; mc++)
        {
            [ForceUnroll]
            for (int mr = 0; mr < MRows; mr++)
            {
                int ri = mc * MRows + mr;
                int ai = a.tileIdx(mr, k);
                int bi = b.tileIdx(k, mc);
                c.tiles[ri] = linalg.coopMatMulAdd<T, false, half, half, T,
                            MemoryScope.Subgroup, 16, 16, 16
                            >(a.tiles[ai], b.tiles[bi], c.tiles[ri]);
            }
        }
    }
}

// Matrix multiply: A(R0×C0) × B(C0×C1) → MatA(R0×C1)
[ForceInline]
[require(cuda)]
internal WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixAccumulator, R0, C1>
matMul<T, int R0, int C0, int C1>(
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixA, R0, C0> a,
    WaveMatrix<half, linalg.CoopMatMatrixUse.MatrixB, C0, C1> b)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
{
    WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixAccumulator, R0, C1> c;

    [ForceUnroll]
    for (int i = 0; i < c.TileCount; i++)
        c.tiles[i].clear();

    matMad<T, R0, C0, C1>(a, b, c);

    return c;
}
