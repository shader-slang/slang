// TiledMMAVulkan: Vulkan backend for tiled MMA using WaveMatrix + shared memory.
//
// Loads data directly into the correct WaveMatrix type (MatA or MatB) via
// shmem or global memory — no changeMajor or transposeSubMatrix needed.
// Uses fromVectorViaShMem, loadWeightTiled, matMul, toVectorViaShMem.
//
// Called by TiledMMAHelper.backward() on Vulkan/SPIR-V targets.
implementing neural;


[require(cooperative_matrix, subgroup_basic)]
internal struct TiledMMAVulkan<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemPool = SharedMemoryPool<ShMemSize>;
    typealias BaseMMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;
    typealias CMShape = BaseMMA.CMShape;

    static const int M = OutputSize;
    static const int K = InputSize;
    static const int N = SubgroupSize;

    static const int NTileRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
    static const int NTileCols = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
    static const int TileSizeInElements = CMShape.SizeAInElements;

    [ForceInline]
    static int tileElementOffset(int tileRow, int tileCol)
    {
        return (tileRow * NTileCols + tileCol) * TileSizeInElements;
    }

    // =========================================================================
    // Forward MMA using WaveMatrix types.
    //
    // Weight → MatA(M, K) via loadWeightTiled from global.
    // Input → MatB(K, N) via fromVectorViaShMem.
    // Result MatC(M, N) → output vector via toVectorViaShMem.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>
    forward<Address, bool Bias = false>(
        WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
    {
        uint shBase = 0u;

        // Weight: MatA(M, K) from global (row-major tiled layout)
        MatrixA<M, K> wA;
        wA.loadWeightTiled<T, Address, false>(weightAddress);

        // Input: MatB(K, N) — each lane's K-element input as a row
        T inputArr[K];
        for (int i = 0; i < K; i++) inputArr[i] = input[i];
        MatrixB<K, N> inputB;
        inputB.fromVectorViaShMem<ShMemSize, T, K, false>(inputArr, shBase);

        // MMA: result(M, N) = W(M, K) × input(K, N)
        MatrixC<T, M, N> resultMat = matMul<T, M, K, N>(wA, inputB);

        // Extract output vector
        typealias OutputVec = WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>;
        T outputArr[M] = {};
        resultMat.toVectorViaShMem<ShMemSize, M, false>(outputArr, shBase);

        if (Bias)
        {
            for (int i = 0; i < M; i++)
                outputArr[i] = outputArr[i] + biasAddress.value[uint(i)];
        }

        return OutputVec(outputArr);
    }

    // =========================================================================
    // Transpose MMA: result = W^T × input
    //
    // Weight tiles loaded from global into MatA(K, M) with transposed addressing.
    // Input loaded from per-lane vector into MatB(M, N) via shmem.
    // Result extracted from MatC(K, N) via shmem.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void mmaTranspose<Address>(
        MatrixB<M, N> inputB,
        Address weightAddress,
        inout T resultArr[K],
        uint shBase)
        where Address : IPointerLikeAddress<T>
    {
        MatrixA<K, M> wA;
        wA.loadWeightTiled<T, Address, true>(weightAddress);

        // MMA: result(K, N) = W^T(K, M) × input(M, N)
        MatrixC<T, K, N> resultMat = matMul<T, K, M, N>(wA, inputB);

        resultMat.toVectorViaShMem<ShMemSize, K, false>(resultArr, shBase);
    }

    // =========================================================================
    // Bias reduce: dBias += sum(dOut) across lanes
    //
    // Reuses the pre-loaded dOutB (MatB<M, N>) directly — no shmem reload.
    // MMA: ones(16,16) × dOutB_tile → column sums. Then cross-warp reduce + atomicAdd.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void biasReduce<Address>(
        MatrixB<M, N> dOutB,
        int subgroupId,
        Address biasGradAddress,
        uint shBase)
        where Address : IPointerLikeAddress<T>
    {
        typealias Tile16A = linalg.CoopMat<half, MemoryScope.Subgroup, 16, 16, linalg.CoopMatMatrixUse.MatrixA>;
        typealias Tile16C = linalg.CoopMat<T, MemoryScope.Subgroup, 16, 16, linalg.CoopMatMatrixUse.MatrixAccumulator>;

        const uint laneId = WaveGetLaneIndex();
        const int subgroupCount = SubgroupCount;

        static const int NOutputTiles = (M + 15) / 16;
        static const int NLaneTiles = (N + 15) / 16;
        static const int TileStrideVec = 16 / CMShape.ElementCountPerVectorMatC;
        static const int MPadded = (M + 1) & ~1;
        static const int ColsPacked = MPadded / 2;
        static const int H2PerTile = 16 / 2;

        // Shmem layout: temp area for matC store, then cross-warp reduction area
        static const int TempAreaPerWarp = TileStrideVec * 16;
        uint tempBase = shBase + uint(subgroupId) * uint(TempAreaPerWarp);
        uint crossWarpBase = shBase + uint(subgroupCount) * uint(TempAreaPerWarp);

        Tile16A matOnes;
        matOnes.fill(half(1));

        [ForceUnroll]
        for (int ot = 0; ot < NOutputTiles; ot++)
        {
            // MMA: sum across lane tiles using pre-loaded dOutB
            Tile16C matC;
            matC.fill(T(0));

            [ForceUnroll]
            for (int g = 0; g < NLaneTiles; g++)
            {
                int bIdx = dOutB.tileIdx(ot, g);
                matC = linalg.coopMatMulAdd<T, false, half, half, T,
                    MemoryScope.Subgroup, 16, 16, 16>(matOnes, dOutB.tiles[bIdx], matC);
            }

            // All rows of matC are identical. Store to temp shmem, copy row 0 to cross-warp layout.
            static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;
            matC.Store<RowMajor, uint4>(ShMemPool.data, tempBase, uint(TileStrideVec));

            if (sizeof(T) == 2)
            {
                int dstBase = ot * H2PerTile;
                int h2ToCopy = ((ColsPacked - dstBase) < H2PerTile) ? (ColsPacked - dstBase) : H2PerTile;
                for (int p = int(laneId); p < h2ToCopy; p += N)
                {
                    vector<T, 2> v2 = ShMemPool.smemLoad<vector<T, 2>>(tempBase, p);
                    ShMemPool.smemStore(crossWarpBase, subgroupId * ColsPacked + dstBase + p, v2);
                }
            }
            else if (sizeof(T) == 4)
            {
                int dstBase = ot * 16;
                int elemsToCopy = ((MPadded - dstBase) < 16) ? (MPadded - dstBase) : 16;
                for (int p = int(laneId); p < elemsToCopy; p += N)
                {
                    T val = ShMemPool.smemLoad<T>(tempBase, p);
                    ShMemPool.smemStore(crossWarpBase, subgroupId * MPadded + dstBase + p, val);
                }
            }
        }

        GroupMemoryBarrierWithGroupSync();

        // Cross-warp reduction and atomic write
        BaseMMA.crossWarpReduceAndAtomicWrite<T, Address, MPadded, ColsPacked>(
            crossWarpBase, subgroupId, subgroupCount, biasGradAddress);
    }

    // =========================================================================
    // Outer product: dW += dOut ⊗ input (summed across lanes via MMA)
    //
    // dOut loaded as MatA(M, N) — each lane's dOut as a column.
    // input loaded as MatB(N, K) — each lane's input as a row.
    // MMA: dW(M, K) = dOut^T(M, N) × input(N, K)
    // Cross-warp reduction via storeRaw/loadRaw/add, then atomic store.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void outerProductAccumulate<Address>(
        T dOutArr[M],
        T inputArr[K],
        Address weightGradAddress,
        uint shBase)
        where Address : IPointerLikeAddress<T>
    {
        const uint subgroupIndex = getWaveId();

        // dOut: MatA(M, N) — each lane's dOut as a column, converted to half internally
        MatrixA<M, N> dOutA;
        dOutA.fromVectorViaShMem<ShMemSize, T, M, false>(dOutArr, shBase);

        // input: MatB(N, K) — each lane's input as a row
        static const int ElemPerUint4_half = sizeof(uint4) / sizeof(half);
        static const int ShmemPerWarpA = N * ((M + ElemPerUint4_half - 1) / ElemPerUint4_half);
        uint shInputBase = shBase + uint(SubgroupCount) * uint(ShmemPerWarpA);

        MatrixB<N, K> inputB;
        inputB.fromVectorViaShMem<ShMemSize, T, K, true>(inputArr, shInputBase);

        GroupMemoryBarrierWithWaveSync();

        // MMA: dW(M, K) = dOut^T(M, N) × input(N, K)
        typealias OPResult = MatrixC<T, M, K>;
        OPResult dW = matMul<T, M, N, K>(dOutA, inputB);

        // Cross-warp reduction via storeRaw/loadRaw/add (portable)
        if (SubgroupCount > 1)
        {
            static const int OPTileStride = OPResult.TileStride;
            static const int OPTotalTiles = OPResult.TileCount;

            for (int s = 1; s < SubgroupCount; s <<= 1)
            {
                uint subgroupOffset = (subgroupIndex / uint(s << 1)) * uint(OPTotalTiles * OPTileStride);
                uint base = shBase + subgroupOffset;

                if (subgroupIndex % uint(s << 1) == uint(s))
                    dW.storeRaw<ShMemSize>(base);

                GroupMemoryBarrierWithGroupSync();

                if (subgroupIndex % uint(s << 1) == 0)
                {
                    if (subgroupIndex != uint(SubgroupCount - 1))
                    {
                        OPResult other;
                        other.loadRaw<ShMemSize>(base);
                        dW = dW.add(other);
                    }
                }

                GroupMemoryBarrierWithGroupSync();
            }
        }

        // Store dW tiles to global via shmem + atomicAdd (tiled layout)
        if (subgroupIndex == 0)
        {
            static const uint SingleTileStrideInUint4 = CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;
            static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;

            [ForceUnroll]
            for (uint i = 0; i < NTileRows; i++)
            {
                [ForceUnroll]
                for (uint j = 0; j < NTileCols; j++)
                {
                    int tileIdx = int(j) * NTileRows + int(i);
                    uint tileOffset = uint(j) * CMShape.CoopMatCSizeInVector;
                    dW.tiles[tileIdx].Store<RowMajor, uint4>(
                        ShMemPool.data, shBase + tileOffset, SingleTileStrideInUint4);
                }
                GroupMemoryBarrierWithWaveSync();

                storeTileRowToGlobal<T, Address>(shBase, int(i), weightGradAddress);
            }
        }

        GroupMemoryBarrierWithGroupSync();
    }

    // =========================================================================
    // Backward: orchestrates mma + biasReduce + outerProductAccumulate
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static WaveTangledVector<T.Differential, ShMemSize, InputSize, SubgroupSize, SubgroupCount>
    backward<Address>(
        WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount> dOut,
        Address weightAddress,
        Address weightGradAddress,
        Address biasGradAddress)
        where Address : IPointerLikeAddress<T>
    {
        uint shBase = 0u;

        // Prepare T-typed arrays from WaveTangledVectors
        T dOutArr[M];
        for (int i = 0; i < M; i++) dOutArr[i] = dOut[i];
        T inputArr[K];
        for (int i = 0; i < K; i++) inputArr[i] = input[i];

        // Load dOut as MatB(M, N) once — reused by both MMA and bias reduce.
        // fromVectorViaShMem handles float→half conversion internally.
        MatrixB<M, N> dOutB;
        dOutB.fromVectorViaShMem<ShMemSize, T, M, false>(dOutArr, shBase);

        // 1. Transpose MMA: dInput = W^T × dOut
        T dInputArr[K] = {};
        mmaTranspose<Address>(dOutB, weightAddress, dInputArr, shBase);

        GroupMemoryBarrierWithGroupSync();

        // 2. Bias reduce: dBias += sum(dOut)
        // Reuses dOutB loaded in step 1 — no re-loading from shmem.
        biasReduce<Address>(dOutB, int(getWaveId()), biasGradAddress, shBase);

        GroupMemoryBarrierWithGroupSync();

        // 3. Outer product: dW += dOut ⊗ input
        // fromVectorViaShMem handles float→half conversion internally.
        outerProductAccumulate<Address>(dOutArr, inputArr, weightGradAddress, shBase);


        typealias InputVecDiff = WaveTangledVector<T.Differential, ShMemSize, K, N, SubgroupCount>;
        InputVecDiff dInput;
        for (int i = 0; i < K; i++) dInput.data[i] = T.Differential(dInputArr[i]);
        return dInput;
    }

    // Store one row of tiles from shmem to global using atomicAdd.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void storeTileRowToGlobal<U, Address>(
        uint sharedMemoryOffset,
        int tileRow,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        const uint laneId = WaveGetLaneIndex();

        if (sizeof(U) == 2)
        {
            static const int H2PerTile = TileSizeInElements / 2;
            static const int TotalH2PerRow = NTileCols * H2PerTile;
            static const int ItersPerLane = (TotalH2PerRow + N - 1) / N;

            [ForceUnroll]
            for (uint k = 0; k < ItersPerLane; k++)
            {
                int h2Idx = int(laneId) + int(k) * N;
                int tileCol = h2Idx / H2PerTile;
                int localH2 = h2Idx % H2PerTile;
                int elemOffset = tileElementOffset(tileRow, tileCol) + localH2 * 2;

                vector<U, 2> h2 = ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, h2Idx);
                let addr = weightAddress.getOffset(elemOffset);
                addr.atomicAdd(0, h2);
            }
        }
        else if (sizeof(U) == 4)
        {
            static const int TotalElemsPerRow = NTileCols * TileSizeInElements;
            static const int ItersPerLane = (TotalElemsPerRow + N - 1) / N;

            [ForceUnroll]
            for (uint k = 0; k < ItersPerLane; k++)
            {
                int elemIdx = int(laneId) + int(k) * N;
                int tileCol = elemIdx / TileSizeInElements;
                int localElem = elemIdx % TileSizeInElements;
                int elemOffset = tileElementOffset(tileRow, tileCol) + localElem;
                int shmemIdx = tileCol * TileSizeInElements + localElem;

                U val = ShMemPool.smemLoad<U>(sharedMemoryOffset, shmemIdx);
                weightAddress.atomicAdd(uint(elemOffset), val);
            }
        }
    }
}
