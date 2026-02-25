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
VISIBILITY_LEVEL struct TiledMMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA = false>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemPool = SharedMemoryPool<ShMemSize>;

    // Re-use the base MMA infrastructure for operations that don't change
    // (input-vector loading, output-vector store-back, cross-warp reduction).
    typealias BaseMMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA>;

    VISIBILITY_LEVEL typealias TileInfoT = BaseMMA.TileInfo;
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

    // Slow path: load a column of tiles (all NCoopMatRow tiles for a given tileCol)
    // into shared memory. Needed when U != half, because CoopMat A only supports
    // half and the data must be converted.
    // Shared memory layout: tiles are stored contiguously, tile i at offset
    // i * TileSizeInUint4, matching what MatA.Load expects with stride TileStrideInUint4.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void loadTileColumnViaSharedMemory<U, Address>(
        uint sharedMemoryOffset,
        int tileCol,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        uint laneId = WaveGetLaneIndex();

        // No bounds check needed: TotalUint4 is always a multiple of N (SubgroupSize).
        // Proof: TileSizeInUint4 = ROW_A * COLUMN_A / ElementCountPerVector
        //      = 16 * 16 / 8 = 32, and N = SubgroupSize = 32.
        // So TotalUint4 = 32 * NTileRows, and NumLoadsPerThread = NTileRows exactly.
        // The maximum idx = laneId + (NTileRows - 1) * 32 ≤ 31 + (NTileRows - 1) * 32
        //                 = NTileRows * 32 - 1 = TotalUint4 - 1, always in bounds.
        static const uint TotalUint4 = TileSizeInUint4 * NTileRows;
        static const uint NumLoadsPerThread = TotalUint4 / N;

        [ForceUnroll]
        for (uint i = 0; i < NumLoadsPerThread; i++)
        {
            uint idx = laneId + i * N;
            uint tileRow = idx / TileSizeInUint4;
            uint idxInTile = idx % TileSizeInUint4;
            int baseOffset = tileElementOffset(tileRow, tileCol);
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
    internal static MatA loadTileDirect<U, Address>(
        int tileRow,
        int tileCol,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        int offset = tileElementOffset(tileRow, tileCol);
        // U is guaranteed to be half at the call site, so U* == half*.
        half* ptr = reinterpret<half*>(weightAddress.getPointer(uint(offset)));
        // CoopMat.Load(T* buffer, uint element, uint stride) — element is 0
        // because the pointer already points to the tile start.
        return MatA.Load<RowMajor>(ptr, 0u, uint(CMShape.COLUMN_A));
    }

    // =========================================================================
    // Tile store helper
    // =========================================================================

    // Store a single tile from shared memory to its tiled-layout position in
    // global memory using atomic writes (for concurrent gradient accumulation).
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

        // No bounds check needed: TileSizeInUint4C is always a multiple of N (SubgroupSize).
        // Proof: TileSizeInUint4C = ROW_C * COLUMN_C / ElementCountPerVectorMatC
        //      = 16 * 16 / (sizeof(uint4) / sizeof(T)).
        //   half:  256 / 8 = 32  → 32 / 32 = 1 store per thread
        //   float: 256 / 4 = 64  → 64 / 32 = 2 stores per thread
        // The maximum idx = laneId + (NumStoresPerThread - 1) * 32
        //                 = TileSizeInUint4C - 1, always in bounds.
        static const int TileSizeInUint4C = TileSizeInElements / CMShape.ElementCountPerVectorMatC;
        static const uint NumStoresPerThread = TileSizeInUint4C / N;

        [ForceUnroll]
        for (uint i = 0; i < NumStoresPerThread; i++)
        {
            uint idx = laneId + i * N;
            uint4 value = ShMemPool.data[sharedMemoryOffset + idx];
            int elemOffset = baseOffset + idx * CMShape.ElementCountPerVectorMatC;
            weightAddress.writeUint4Atomic<T, true, CMShape.COLUMN_C>(elemOffset, value);
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
    [require(cooperative_matrix, subgroup_basic, cuda_spirv)]
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
        [ForceUnroll]
        for (int i = 0; i < TileInfoT.NCoopMat; i++)
        {
            matC[i].fill(T(0.0f));
        }

        OutArrayType outputVector;
        if (Bias)
        {
            [ForceUnroll]
            for (int i = 0; i < M; i++)
            {
                U bias = biasAddress.value[i];
                outputVector[i] = __realCast<V>(bias);
            }
        }

        [ForceUnroll]
        for (int k = 0; k < K; k += CMShape.COLUMN_A)
        {
            uint tileCol = k / CMShape.COLUMN_A;

            MatA matA[TileInfoT.NCoopMatRow];

            if (U is half)
            {
                // Fast path: each warp loads weight tiles directly from global memory,
                // then loads its input vector into per-warp shared memory B.
                // Only a warp-level sync is needed (for shared memory B visibility).

                // Phase 1: Load weight tiles into CoopMat A.
                [ForceUnroll]
                for (uint i = 0; i < TileInfoT.NCoopMatRow; i++)
                {
                    matA[i] = loadTileDirect<U, Address>(i, tileCol, weightAddress);
                }

                // Phase 2: Load input vector into shared memory B.
                BaseMMA.loadVectorToShB(sharedMemoryBOffset, tileCol, subgroupIndex, inputVector);
                GroupMemoryBarrierWithWaveSync();
            }
            else
            {
                // Slow path: first warp loads the entire column of tiles to
                // workgroup-shared memory for type conversion. All warps load
                // their input vectors in parallel. A single GroupSync covers both.

                // Phase 1: Load weight tiles into shared memory first, because we can't
                // load them directly into CoopMat A.
                if (subgroupIndex == 0)
                {
                    loadTileColumnViaSharedMemory<U, Address>(sharedMemoryAOffset, tileCol, weightAddress);
                }

                // Phase 2: Load input vector into shared memory B.
                BaseMMA.loadVectorToShB(sharedMemoryBOffset, tileCol, subgroupIndex, inputVector);
                GroupMemoryBarrierWithGroupSync();

                [ForceUnroll]
                for (uint i = 0; i < TileInfoT.NCoopMatRow; i++)
                {
                    uint ptrTileOffset = sharedMemoryAOffset + i * TileSizeInUint4;
                    matA[i] = MatA.Load<RowMajor, uint4>(ShMemPool.data, ptrTileOffset, TileStrideInUint4);
                }
            }

            // Phase 3: MMA.
            uint ptrPerWarpBOffset = sharedMemoryBOffset + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatB;
            [ForceUnroll]
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

        [ForceUnroll]
        for (int i = 0; i < TileInfoT.NCoopMatRow; i++)
        {
            [ForceUnroll]
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

    // =========================================================================
    // Backward outer-product with tiled-layout store
    // =========================================================================

    /// Outer product dW = dOut x input^T stored directly in tiled layout.
    ///
    /// The computation and cross-warp reduction are identical to
    /// MMAHelper.outerProductAccumulate.  Only the store phase differs:
    /// cooperative-matrix tiles are written directly to their tiled-layout
    /// positions in global memory (no conversion back to linear layout).
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic, cuda_spirv)]
    VISIBILITY_LEVEL static void outerProductAccumulateTiledLayout<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
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

        static const int BatchSize = CMShape.COLUMN_A;
        static const int CoopMatCRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
        static const int CoopMatCColumns = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
        static const int StrideInVectorTileB = CoopMatCColumns * CMShape.COLUMN_B / CMShape.ElementCountPerVector;
        static const int StrideInVectorTileA = CoopMatCRows * CMShape.ROW_A / CMShape.ElementCountPerVector;

        MatC matC[CoopMatCRows * CoopMatCColumns];
        [ForceUnroll]
        for (int i = 0; i < CoopMatCRows * CoopMatCColumns; i++)
        {
            matC[i].fill(T(0.0f));
        }

        uint warpLocalPtrAOffset = sharedMemoryAOffset + subgroupIndex * StrideInVectorTileA * BatchSize;
        uint warpLocalPtrBOffset = sharedMemoryBOffset + subgroupIndex * StrideInVectorTileB * BatchSize;

        // Outer product computation (identical to MMAHelper.outerProductAccumulate).
        [ForceUnroll]
        for (uint k = 0; k < N; k += CMShape.COLUMN_A)
        {
            uint tileIndex = k / CMShape.COLUMN_A;
            BaseMMA.loadVectorForOuterProduct<BatchSize, TileInfoT.Uint4AlignedM / CMShape.ElementCountPerVector, StrideInVectorTileA, TypeA, InArrayTypeA>(warpLocalPtrAOffset, tileIndex, inputVectorA);
            BaseMMA.loadVectorForOuterProduct<BatchSize, TileInfoT.Uint4AlignedK / CMShape.ElementCountPerVector, StrideInVectorTileB, TypeB, InArrayTypeB>(warpLocalPtrBOffset, tileIndex, inputVectorB);
            GroupMemoryBarrierWithWaveSync();

            MatA matA[CoopMatCRows];
            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                matA[i] = MatA.Load<ColumnMajor, uint4>(ShMemPool.data, warpLocalPtrAOffset + i * CMShape.ROW_A / CMShape.ElementCountPerVector, StrideInVectorTileA);
            }

            [ForceUnroll]
            for (uint j = 0; j < CoopMatCColumns; j++)
            {
                let matB = MatB.Load<RowMajor, uint4>(ShMemPool.data, warpLocalPtrBOffset + j * CMShape.COLUMN_B / CMShape.ElementCountPerVector, StrideInVectorTileB);
                for (uint i = 0; i < CoopMatCRows; i++)
                {
                    int index = i * CoopMatCColumns + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        // Cross-warp reduction (same as MMAHelper.outerProductAccumulate).
        BaseMMA.sumReduceTiles<CoopMatCRows, CoopMatCColumns>(matC, subgroupIndex, sharedMemoryBOffset);

        // Store result in tiled layout.  Only subgroup 0 writes to global memory.
        // For each row of tiles: store all columns contiguously to shared memory,
        // then write all tiles in that row to global memory in one pass.
        if (subgroupIndex == 0)
        {
            static const uint SingleTileStrideInUint4 = CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;

            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                // Store one row of CoopMat tiles contiguously into shared memory.
                // Tile j at offset j * CoopMatCSizeInVector, each tile stored with
                // its own row stride (SingleTileStrideInUint4).
                [ForceUnroll]
                for (uint j = 0; j < CoopMatCColumns; j++)
                {
                    matC[i * CoopMatCColumns + j].Store<RowMajor, uint4>(
                        ShMemPool.data, sharedMemoryBOffset + j * CMShape.CoopMatCSizeInVector, SingleTileStrideInUint4);

                    GroupMemoryBarrierWithWaveSync();

                    storeTileToGlobal<U, Address>(sharedMemoryBOffset, i, j, weightAddress);

                    GroupMemoryBarrierWithWaveSync();
                }
            }
        }

        AllMemoryBarrierWithGroupSync();
    }
}
