// Tiled-layout (OptimalLayout) MMA helpers.
//
// This file implements cooperative-matrix MMA and outer-product operations
// assuming the weight matrix is pre-tiled into 16x16 cooperative-matrix-sized
// tiles.  Moving this to a separate file keeps accelerate-vector-coopmat.slang
// focused on the linear-layout path.
implementing neural;

#include "common-def.slang"

// Round up a compile-time integer to the next multiple of 16 (cooperative matrix tile size).
internal struct TileAlignedSize<int Size>
{
    internal static const int Value = ((Size + 15) / 16) * 16;
}

/// Cooperative-matrix operations with tiled (OptimalLayout) weight storage.
///
/// Tiled layout:
///   The M x K weight matrix is divided into NTileRows x NTileCols tiles, each
///   ROW_A x COLUMN_A (16x16) elements.  Tiles are stored in row-major tile order:
///     tile(tileRow, tileCol) at linear index = tileRow * NTileCols + tileCol
///   Within each tile, elements are contiguous in row-major order.
///   **M and K must be multiples of 16.**
///
/// Key advantages over the linear-layout MMAHelper path:
///   * When U is half, weight tiles are loaded directly from global memory
///     into CoopMat registers — no shared memory for A, no cross-warp GroupSync.
///   * When U is float (needs half conversion), tiles are staged through
///     per-warp shared memory with no padding or bounds-checking.
///   * Outer-product results are stored directly in tiled layout without
///     converting back to linear format.
[require(cooperative_matrix)]
VISIBILITY_LEVEL struct TiledMMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemPool = SharedMemoryPool<ShMemSize>;

    // Re-use the base MMA infrastructure for operations that don't change
    // (input-vector loading, output-vector store-back, cross-warp reduction).
    typealias BaseMMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;

    VISIBILITY_LEVEL typealias TileInfoT = BaseMMA.TileInfo;
    VISIBILITY_LEVEL typealias TileInfo = TileInfoT;
    typealias CMShape = BaseMMA.CMShape;
    typealias ShMemInfo = BaseMMA.ShMemInfo;

    static const linalg.CoopMatMatrixLayout RowMajor = BaseMMA.RowMajor;
    static const linalg.CoopMatMatrixLayout ColumnMajor = BaseMMA.ColumnMajor;

    static const int M = BaseMMA.M;
    static const int N = BaseMMA.N;
    static const int K = BaseMMA.K;

    typealias MatA = BaseMMA.MatA;
    typealias MatB = BaseMMA.MatB;
    typealias MatC = BaseMMA.MatC;

    // =========================================================================
    // Tiled-layout addressing
    // =========================================================================

    static const int NTileRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
    static const int NTileCols = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;

    static const int TileSizeInElements = CMShape.SizeAInElements;          // 256
    static const int TileSizeInUint4    = CMShape.CoopMatASizeInVector;    // 32

    // Row stride inside a tile, in uint4 units.
    static const int TileStrideInUint4 = CMShape.COLUMN_A / CMShape.ElementCountPerVector; // 2

    // Element offset of tile(tileRow, tileCol) from the weight buffer start.
    // TODO: This just assumes that the tiles are organized in row-major order.
    [ForceInline]
    static int tileElementOffset(int tileRow, int tileCol)
    {
        return (tileRow * NTileCols + tileCol) * TileSizeInElements;
    }

    // =========================================================================
    // Tile load helpers
    // =========================================================================

    // Slow path: load NCoopMatRow tiles for a given shared-dimension index
    // into shared memory. Needed when U != half, because CoopMat A only
    // supports half and the data must be converted.
    //
    // TransposeA=false: loads a column of tiles (varying M, fixed K=tileIndex)
    // TransposeA=true:  loads a row of tiles (fixed M=tileIndex, varying K)
    //
    // Shared memory layout: tiles are stored contiguously, tile i at offset
    // i * TileSizeInUint4.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void loadTileColumnViaSharedMemory<U, Address>(
        uint sharedMemoryOffset,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        uint laneId = WaveGetLaneIndex();

        // NCoopMatRow tiles to load. Each tile is TileSizeInUint4 = 32 uint4.
        // TotalUint4 = 32 * NCoopMatRow, always a multiple of N = 32.
        static const uint NumTiles = TileInfoT.NCoopMatRow;
        static const uint TotalUint4 = TileSizeInUint4 * NumTiles;
        static const uint NumLoadsPerThread = TotalUint4 / N;

        for (uint i = 0; i < NumLoadsPerThread; i++)
        {
            uint idx = laneId + i * N;
            uint localTileIdx = idx / TileSizeInUint4;
            uint idxInTile = idx % TileSizeInUint4;

            // Map logical tile index to stored M×K coordinates.
            // TransposeA=false: stored(localTileIdx, tileIndex) — localTileIdx is M, tileIndex is K
            // TransposeA=true:  stored(tileIndex, localTileIdx) — tileIndex is M, localTileIdx is K
            int storedRow = TransposeA ? tileIndex : int(localTileIdx);
            int storedCol = TransposeA ? int(localTileIdx) : tileIndex;

            int baseOffset = tileElementOffset(storedRow, storedCol);
            int elemOffset = baseOffset + idxInTile * CMShape.ElementCountPerVector;
            uint4 value = weightAddress.readUint4<half, true, CMShape.COLUMN_A>(elemOffset);
            ShMemPool.data[sharedMemoryOffset + idx] = value;
        }
    }

    // Fast path: load a CoopMat A tile directly from global memory.
    // The data is already half — the only type MatA supports — so
    // CoopMat.Load can read straight from global memory with no shared
    // memory, no type conversion, and no cross-warp sync.
    // Caller must ensure U is half before calling.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    // Load a CoopMat A tile directly from global memory.
    // tileRowIdx and tileColIdx are in the iteration's logical space
    // (tileRowIdx = NCoopMatRow index, tileColIdx = shared dimension index).
    // This function maps them to the stored M×K tile layout accounting for TransposeA.
    internal static MatA loadTileDirect<U, Address>(
        int tileRowIdx,
        int tileColIdx,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        // Map logical iteration indices to stored M×K tile coordinates.
        // TransposeA=false: i indexes M (tileRow), tileIndex indexes K (tileCol)
        // TransposeA=true:  i indexes K (tileCol), tileIndex indexes M (tileRow)
        int storedRow = TransposeA ? tileColIdx : tileRowIdx;
        int storedCol = TransposeA ? tileRowIdx : tileColIdx;

        int offset = tileElementOffset(storedRow, storedCol);
        let tileAddr = weightAddress.getOffset(offset);

        if (TransposeA)
        {
            // Tile is stored row-major in M×K, but we need it column-major for W^T.
            // ColumnMajor with stride ROW_A transposes the tile on load.
            return bit_cast<MatA>(loadCoopMat<U, Address, CMShape.ROW_A, CMShape.COLUMN_A, linalg.CoopMatMatrixUse.MatrixA, ColumnMajor>(
                tileAddr, 0u, uint(CMShape.ROW_A)));
        }
        else
        {
            return bit_cast<MatA>(loadCoopMat<U, Address, CMShape.ROW_A, CMShape.COLUMN_A, linalg.CoopMatMatrixUse.MatrixA, RowMajor>(
                tileAddr, 0u, uint(CMShape.COLUMN_A)));
        }
    }

    // =========================================================================
    // Tile store helper
    // =========================================================================

    // Store a single tile from shared memory to its tiled-layout position in
    // global memory using atomic writes (for concurrent gradient accumulation).
    // Writes in the native element type U (half or float) — no precision conversion.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void storeTileToGlobal<U, Address>(
        uint sharedMemoryOffset,
        int tileRow,
        int tileCol,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        uint laneId = WaveGetLaneIndex();
        int baseOffset = tileElementOffset(tileRow, tileCol);

        static const int TileSizeInUint4C_Half = TileSizeInElements / 8;   // 32 uint4 for half
        static const int HalfsPerUint4 = 8;

        if (sizeof(U) == 2)
        {
            for (int v = (int)laneId; v < TileSizeInUint4C_Half; v += N)
            {
                uint4 value = ShMemPool.data[sharedMemoryOffset + v];
                let globalAddr = weightAddress.getOffset(baseOffset + v * HalfsPerUint4);
                globalAddr.writeUint4Atomic<half, true, CMShape.COLUMN_C>(0, value);
            }
        }
        else if (sizeof(U) == 4)
        {
            static const int FloatsPerUint4 = 4;
            static const int TileSizeInUint4C_Float = TileSizeInElements / FloatsPerUint4;

            for (int v = (int)laneId; v < TileSizeInUint4C_Float; v += N)
            {
                uint4 value = ShMemPool.data[sharedMemoryOffset + v];
                let globalAddr = weightAddress.getOffset(baseOffset + v * FloatsPerUint4);
                globalAddr.writeUint4Atomic<U, true, CMShape.COLUMN_C>(0, value);
            }
        }
    }

    // =========================================================================
    // Forward MMA with tiled weight layout
    // =========================================================================

    /// Matrix-vector multiply with tiled weight layout.
    ///
    /// Key optimisations over MMAHelper.mma:
    ///  * When U is half: weight tiles are loaded directly from global memory
    ///    into CoopMat registers. No shared memory for A, no GroupSync.
    ///  * When U is not half: tiles are staged through workgroup-shared memory
    ///    (sharedMemoryA) for float -> half conversion. Only the first warp
    ///    loads the tile, then GroupMemoryBarrierWithGroupSync ensures all
    ///    warps see the data before loading into CoopMat registers.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static OutArrayType mma<U, Address, InArrayType, OutArrayType, V, bool Bias = false>(
        InArrayType inputVector,
        uint sharedMemoryAOffset,
        uint sharedMemoryBOffset,
        uint sharedMemoryCOffset,
        Address weightAddress,
        Optional<Address> biasAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
            where V : __BuiltinFloatingPointType
            where V.Differential == V
            where InArrayType : IArrayAccessor<V>
            where OutArrayType : IArrayAccessor<V>
    {
        uint ptrCOffset = sharedMemoryCOffset;
        const uint subgroupIndex = getWaveId();

        MatC matC[TileInfoT.NCoopMat];
        for (int i = 0; i < TileInfoT.NCoopMat; i++)
        {
            matC[i].fill(T(0.0f));
        }

        OutArrayType outputVector;
        if (Bias)
        {
            for (int i = 0; i < M; i++)
            {
                U bias = biasAddress.value[i];
                outputVector[i] = __realCast<V>(bias);
            }
        }

        [ForceUnroll]
        for (int k = 0; k < TileInfoT.SharedDimensionSize; k += CMShape.COLUMN_A)
        {
            uint tileIndex = k / CMShape.COLUMN_A;

            MatA matA[TileInfoT.NCoopMatRow];

            if (U is half)
            {
                // Fast path: each warp loads weight tiles directly from global memory,
                // then loads its input vector into per-warp shared memory B.
                // Only a warp-level sync is needed (for shared memory B visibility).

                // Phase 1: Load weight tiles into CoopMat A.
                for (uint i = 0; i < TileInfoT.NCoopMatRow; i++)
                {
                    matA[i] = loadTileDirect<U, Address>(i, tileIndex, weightAddress);
                }

                // Phase 2: Load input vector into shared memory B.
                BaseMMA.loadVectorToShB(sharedMemoryBOffset, tileIndex, subgroupIndex, inputVector);
                GroupMemoryBarrierWithWaveSync();
            }
            else
            {
                // Slow path: first warp loads the tiles to workgroup-shared memory
                // for type conversion. All warps load their input vectors in parallel.
                if (subgroupIndex == 0)
                {
                    loadTileColumnViaSharedMemory<U, Address>(sharedMemoryAOffset, tileIndex, weightAddress);
                }

                BaseMMA.loadVectorToShB(sharedMemoryBOffset, tileIndex, subgroupIndex, inputVector);
                GroupMemoryBarrierWithGroupSync();

                for (uint i = 0; i < TileInfoT.NCoopMatRow; i++)
                {
                    uint ptrTileOffset = sharedMemoryAOffset + i * TileSizeInUint4;
                    if (TransposeA)
                    {
                        matA[i] = MatA.Load<ColumnMajor, uint4>(ShMemPool.data, ptrTileOffset, TileStrideInUint4);
                    }
                    else
                    {
                        matA[i] = MatA.Load<RowMajor, uint4>(ShMemPool.data, ptrTileOffset, TileStrideInUint4);
                    }
                }
            }

            // Phase 3: MMA.
            // Use SharedMemSizeInVectorMatC (not MatB) for per-warp stride to match
            // the writeback layout, keeping each warp's B/C regions aligned.
            uint ptrPerWarpBOffset = sharedMemoryBOffset + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatC;
            for (uint j = 0; j < TileInfoT.NCoopMatColumn; j++)
            {
                uint ptrMatBOffset = ptrPerWarpBOffset + j * CMShape.CoopMatBSizeInVector;
                let matB = MatB.Load<ColumnMajor, uint4>(ShMemPool.data, ptrMatBOffset, TileInfoT.HeightInVectorTileB);

                for (uint i = 0; i < TileInfoT.NCoopMatRow; i++)
                {
                    int index = i * TileInfoT.NCoopMatColumn + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        // Phase 4: Write result to output vector via shared memory C (same as MMAHelper.mma).
        uint ptrPerWarpCOffset = ptrCOffset + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatC;

        for (int i = 0; i < TileInfoT.NCoopMatRow; i++)
        {
            for (int j = 0; j < TileInfoT.NCoopMatColumn; j++)
            {
                int index = i * TileInfoT.NCoopMatColumn + j;
                uint ptrMatCOffset = ptrPerWarpCOffset + j * CMShape.CoopMatCSizeInVector;
                matC[index].Store<ColumnMajor, uint4>(ShMemPool.data, ptrMatCOffset, TileInfoT.HeightInVectorTileC);
            }

            static const AccessOp biasOp = Bias ? AccessOp.ACCUMULATE : AccessOp.WRITE;
            BaseMMA.storeVectorFromShB<V, OutArrayType, biasOp>(ptrCOffset, i, subgroupIndex, outputVector);

            GroupMemoryBarrierWithWaveSync();
        }

        return outputVector;
    }



    // Forward sumReduceRows to BaseMMA.
    [ForceInline]
    [require(cuda_spirv, subgroup_partitioned)]
    [require(cuda_spirv, sm_6_6)]
    VISIBILITY_LEVEL static void sumReduceRows<U, Address, InArrayType>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        BaseMMA.sumReduceRows<U, Address, InArrayType>(sharedMemoryOffset, dOut, subgroupId, biasAddress);
    }

    // =========================================================================
    // Backward outer-product with tiled-layout store
    // =========================================================================

    /// Outer product dW = dOut x input^T stored directly in tiled layout.
    ///
    /// The computation and cross-warp reduction are identical to
    /// MMAHelper.outerProductAccumulate.  Only the store phase differs:
    /// cooperative-matrix tiles are written directly to their tiled-layout
    /// positions in global memory (no conversion back to linear layout).
    // TODO: U and T is actually always the same type, we should clean this up.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    [require(spvAtomicFloat16AddEXT)]
    VISIBILITY_LEVEL static void outerProductAccumulate<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        uint sharedMemoryAOffset,
        uint sharedMemoryBOffset,
        InArrayTypeA inputVectorA,
        InArrayTypeB inputVectorB,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
            where TypeA : __BuiltinFloatingPointType
            where TypeA.Differential == TypeA
            where TypeB : __BuiltinFloatingPointType
            where TypeB.Differential == TypeB
            where InArrayTypeA : IArrayAccessor<TypeA>
            where InArrayTypeB : IArrayAccessor<TypeB>
    {
        const uint subgroupIndex = getWaveId();

        static const int CoopMatCRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
        static const int CoopMatCColumns = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;

        MatC matC[CoopMatCRows * CoopMatCColumns];
        [ForceUnroll]
        for (int i = 0; i < CoopMatCRows * CoopMatCColumns; i++)
            matC[i].fill(T(0.0f));

        BaseMMA.outerProduct<CoopMatCRows, CoopMatCColumns, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
            sharedMemoryAOffset, sharedMemoryBOffset, subgroupIndex, inputVectorA, inputVectorB, matC);

        GroupMemoryBarrierWithGroupSync();
        BaseMMA.sumReduceTiles<CoopMatCRows, CoopMatCColumns>(matC, subgroupIndex, sharedMemoryBOffset);

        // Store result in tiled layout. Row-at-a-time; write-back uses direct atomicAdd.
        if (subgroupIndex == 0)
        {
            static const uint SingleTileStrideInUint4 = CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;
            const uint laneId = WaveGetLaneIndex();

            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                [ForceUnroll]
                for (uint j = 0; j < CoopMatCColumns; j++)
                {
                    uint tileOffset = j * CMShape.CoopMatCSizeInVector;
                    matC[i * CoopMatCColumns + j].Store<RowMajor, uint4>(
                        ShMemPool.data, sharedMemoryBOffset + tileOffset, SingleTileStrideInUint4);
                }
                GroupMemoryBarrierWithWaveSync();

                if (sizeof(T) == 2)
                {
                    // T = half: process 2 elements at a time via native half2 atomicAdd.
                    static const int H2PerTile = TileSizeInElements / 2;
                    static const int TotalH2PerRow = CoopMatCColumns * H2PerTile;
                    static const int ItersPerLane = (TotalH2PerRow + N - 1) / N;

                    [ForceUnroll]
                    for (uint k = 0; k < ItersPerLane; k++)
                    {
                        int h2Idx = (int)laneId + (int)k * N;
                        int tileCol = h2Idx / H2PerTile;
                        int localH2 = h2Idx % H2PerTile;
                        int elemOffset = tileElementOffset((int)i, tileCol) + localH2 * 2;

                        vector<U, 2> h2 = ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryBOffset, h2Idx);
                        let addr = weightAddress.getOffset(elemOffset);
                        addr.atomicAdd(0, h2);
                    }
                }
                else if (sizeof(T) == 4)
                {
                    // T = float: one element per thread, one scalar atomicAdd each.
                    static const int TotalElemsPerRow = CoopMatCColumns * TileSizeInElements;
                    static const int ItersPerLane = (TotalElemsPerRow + N - 1) / N;

                    [ForceUnroll]
                    for (uint k = 0; k < ItersPerLane; k++)
                    {
                        int elemIdx = (int)laneId + (int)k * N;
                        int tileCol = elemIdx / TileSizeInElements;
                        int localElem = elemIdx % TileSizeInElements;
                        int elemOffset = tileElementOffset((int)i, tileCol) + localElem;
                        int shmemIdx = tileCol * TileSizeInElements + localElem;

                        U val = ShMemPool.smemLoad<U>(sharedMemoryBOffset, shmemIdx);
                        weightAddress.atomicAdd(uint(elemOffset), val);
                    }
                }
            }
        }

        GroupMemoryBarrierWithGroupSync();
    }
}
