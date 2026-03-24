implementing neural;

#include "common-def.slang"

// MMAHelperNew: Tiled MMA using m16n8k16 with no shared memory at all.
//
// MatA (weight): loaded directly from tiled global memory into CoopMat registers.
//   The weight matrix must be pre-tiled into 16×16 tiles in row-major tile order.
//   M and K must be multiples of 16.
//
// MatB (input): constructed from each thread's local input vector via FromLocalColumn.
//   Input is read directly from a StructuredBuffer per K-tile to avoid register pressure.
//
// MatC (output): extracted to each thread's local output vector via ToLocalColumn.
//
// MMA shape: m16n8k16
//   MatA: 16×16 (half)
//   MatB: 16×8  (half)
//   MatC: 16×8  (T)
//
// For a 32-thread warp with N_tile=8: 4 MatB/MatC tiles cover all 32 threads.
//
// TransposeA:
//   When true, computes W^T × input instead of W × input.
//   LoadNative loads the tile in its stored (row-major) layout, then ChangeMajor()
//   transposes the fragment registers. The tile addressing swaps row/col indices
//   to walk the stored M×K grid in the transposed order.

[require(cuda)]
VISIBILITY_LEVEL struct MMAHelperNew<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, bool TransposeA = false>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    // m16n8k16 shape constants
    static const int ROW_A = 16;
    static const int COLUMN_A = 16;
    static const int ROW_B = 16;
    static const int COLUMN_B = 8;
    static const int ROW_C = 16;
    static const int COLUMN_C = 8;

    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);

    // Forward: NCoopMatRow tiles down M (output dimension).
    // Transpose: NCoopMatRow tiles down K (output dimension of W^T).
    VISIBILITY_LEVEL static const int NCoopMatRow = TransposeA ? (K + ROW_A - 1) / ROW_A
                                                               : (M + ROW_A - 1) / ROW_A;
    VISIBILITY_LEVEL static const int NCoopMatColumn = (N + COLUMN_B - 1) / COLUMN_B;
    VISIBILITY_LEVEL static const int NCoopMat = NCoopMatRow * NCoopMatColumn;

    VISIBILITY_LEVEL static const int Uint4AlignedK = ((K + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;
    VISIBILITY_LEVEL static const int Uint4AlignedM = ((M + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    // Shared dimension: K for forward, M for transpose.
    static const int SharedDimAligned = TransposeA ? Uint4AlignedM : Uint4AlignedK;
    static const int SharedDimActual  = TransposeA ? M : K;

    // Output dimension: M for forward, K for transpose.
    static const int OutputDim = TransposeA ? K : M;

    static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;
    static const linalg.CoopMatMatrixLayout ColumnMajor = linalg.CoopMatMatrixLayout.ColumnMajor;

    typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, ROW_A, COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, ROW_B, COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T,    MemoryScope.Subgroup, ROW_C, COLUMN_C, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    // =========================================================================
    // Tiled-layout addressing for MatA
    // Physical tile grid is always M×K (the stored weight matrix dimensions).
    // =========================================================================

    static const int NTileRows = (M + ROW_A - 1) / ROW_A;
    static const int NTileCols = (K + COLUMN_A - 1) / COLUMN_A;
    static const int TileSizeInElements = ROW_A * COLUMN_A;

    [ForceInline]
    VISIBILITY_LEVEL static int tileElementOffset(int tileRow, int tileCol)
    {
        return (tileRow * NTileCols + tileCol) * TileSizeInElements;
    }

    [ForceInline]
    [require(cuda)]
    VISIBILITY_LEVEL static MatA loadTileDirect<U, Address>(
        int tileRowIdx,
        int tileColIdx,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        // Map logical iteration indices to physical tile coordinates.
        // Forward:   tileRowIdx indexes M, tileColIdx indexes K → stored(row, col) = (rowIdx, colIdx)
        // Transpose: tileRowIdx indexes K, tileColIdx indexes M → stored(row, col) = (colIdx, rowIdx)
        int storedRow = TransposeA ? tileColIdx : tileRowIdx;
        int storedCol = TransposeA ? tileRowIdx : tileColIdx;

        int offset = tileElementOffset(storedRow, storedCol);
        let tileAddr = weightAddress.getOffset(offset);
        var matA = MatA.LoadNative(bit_cast<RWStructuredBuffer<half>>(tileAddr.getBuffer()), tileAddr.getBaseIndex());

        if (TransposeA)
            matA.ChangeMajor();

        return matA;
    }

    // =========================================================================
    // Tiled MMA: zero shared memory
    // =========================================================================

    [ForceInline]
    [require(cuda)]
    VISIBILITY_LEVEL static OutArrayType mma<U, Address, InArrayType, OutArrayType, V, bool Bias = false>(
        InArrayType inputVector,
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
        MatC matC[NCoopMat];
        [ForceUnroll]
        for (int i = 0; i < NCoopMat; i++)
            matC[i].fill(T(0.0f));

        [ForceUnroll]
        for (int k = 0; k < SharedDimAligned; k += COLUMN_A)
        {
            int tileColIdx = k / COLUMN_A;

            MatA matA[NCoopMatRow];
            [ForceUnroll]
            for (uint i = 0; i < NCoopMatRow; i++)
                matA[i] = loadTileDirect<U, Address>(int(i), tileColIdx, weightAddress);

            half localColB[ROW_B];
            [ForceUnroll]
            for (uint r = 0; r < ROW_B; r++)
            {
                int srcIdx = k + int(r);
                localColB[r] = (srcIdx < SharedDimActual) ? __realCast<half>(inputVector[srcIdx]) : half(0.0);
            }

            for (uint j = 0; j < NCoopMatColumn; j++)
            {
                let matB = MatB.FromLocalColumn(localColB, int(j));

                [ForceUnroll]
                for (uint i = 0; i < NCoopMatRow; i++)
                {
                    int index = i * NCoopMatColumn + int(j);
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        // Extract results and add bias in one pass — no intermediate outputVector array.
        // Each ToLocalColumn gives this thread one column (ROW_C elements).
        // Write directly to the output array, adding bias on the fly.
        OutArrayType outputVector;
        [ForceUnroll]
        for (int i = 0; i < NCoopMatRow; i++)
        {
            for (int j = 0; j < NCoopMatColumn; j++)
            {
                int index = i * NCoopMatColumn + j;

                T localColC[ROW_C];
                matC[index].ToLocalColumn(localColC, j);

                int rowBase = i * ROW_C;
                [ForceUnroll]
                for (int r = 0; r < ROW_C; r++)
                {
                    int outIdx = rowBase + r;
                    if (outIdx < OutputDim)
                    {
                        V val = __realCast<V>(localColC[r]);
                        if (Bias)
                            val = val + __realCast<V>(biasAddress.value[uint(outIdx)]);
                        outputVector[outIdx] = val;
                    }
                }
            }
        }

        return outputVector;
    }

    // =========================================================================
    // Bias gradient reduction: bias_grad[i] = sum_{all lanes} dOut_lane[i]
    //
    // Uses MMA with A=all-ones, B=dOut via FromLocalRow, C=accumulator.
    // FromLocalRow puts thread identity in the K dimension (rows of B),
    // so C = ones × B sums ACROSS THREADS for each output element.
    //
    // Per-warp MMA phase uses zero shared memory.
    // Cross-warp reduction uses shared memory + atomicAdd.
    // =========================================================================

    // FromLocalRow on MatB (16×8): M=16 threads per matrix → 2 groups for 32 threads.
    static const int NRowGroups = (N + ROW_B - 1) / ROW_B;

    // Number of output tiles: M elements in chunks of COLUMN_B (8).
    static const int NBiasTiles = (M + COLUMN_B - 1) / COLUMN_B;

    [ForceInline]
    [require(cuda, subgroup_basic)]
    VISIBILITY_LEVEL static void sumReduceRows<U, Address, InArrayType, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;

        const uint laneId = WaveGetLaneIndex();
        static const int MPadded = (M + 1) & ~1;
        static const int ColsPacked = MPadded / 2;
        static const int H2PerTile = COLUMN_C / 2;

        // MatA = all ones. C = ones × B → C[m][n] = sum_k B[k][n] = col sum of B.
        // FromLocalRow on MatB puts thread identity in K (rows),
        // so col sum = sum across threads. 8 elements per tile (COLUMN_B).
        MatA matOnes;
        matOnes.fill(half(1));

        [ForceUnroll]
        for (int tc = 0; tc < NBiasTiles; tc++)
        {
            half localRowB[COLUMN_B];
            [ForceUnroll]
            for (int n = 0; n < COLUMN_B; n++)
            {
                int elemIdx = tc * COLUMN_B + n;
                localRowB[n] = (elemIdx < M) ? __realCast<half>(dOut[elemIdx]) : half(0.0);
            }

            MatC matC;
            matC.fill(T(0));

            [ForceUnroll]
            for (int g = 0; g < NRowGroups; g++)
            {
                let matB = MatB.FromLocalRow(localRowB, g);
                matC = linalg.coopMatMulAdd<T, false, half, half, T,
                    MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                    >(matOnes, matB, matC);
            }

            // All rows of matC are identical. Extract via ToLocalRow.
            T localResult[COLUMN_C];
            matC.ToLocalRow(localResult, 0);

            // Write per-warp result to shmem using half2-packed stores.
            int dstBase = tc * H2PerTile;
            int remainingH2 = ColsPacked - dstBase;
            int h2ToCopy = (remainingH2 < H2PerTile) ? remainingH2 : H2PerTile;
            for (int p = int(laneId); p < h2ToCopy; p += N)
            {
                vector<U, 2> v2 = vector<U, 2>(
                    __realCast<U>(localResult[p * 2]),
                    __realCast<U>(localResult[p * 2 + 1]));
                ShMemPool.smemStore(sharedMemoryOffset, subgroupId * ColsPacked + dstBase + p, v2);
            }
        }

        GroupMemoryBarrierWithGroupSync();

        // Cross-warp reduction + half2 atomicAdd using all threads.
        {
            const int NUM_VECTORS = SubgroupCount * N;
            const int ColsPerThread = ColsPacked / NUM_VECTORS;
            const int NumIters = (ColsPacked % NUM_VECTORS) != 0 ? ColsPerThread + 1 : ColsPerThread;

            int packed_col_idx = subgroupId * N + int(laneId);

            if (sizeof(U) == 2)
            {
                for (int j = 0; j < NumIters; j++)
                {
                    if (packed_col_idx < ColsPacked)
                    {
                        vector<U, 2> sum_v2 = vector<U, 2>(U(0), U(0));
                        for (int i = 0; i < SubgroupCount; i++)
                            sum_v2 = sum_v2 + ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, packed_col_idx + i * ColsPacked);

                        int elemIdx0 = packed_col_idx * 2;
                        int elemIdx1 = elemIdx0 + 1;

                        if (elemIdx1 < M)
                        {
                            let addr = biasAddress.getOffset(elemIdx0);
                            addr.atomicAdd(0u, sum_v2);
                        }
                        else if (elemIdx0 < M)
                        {
                            biasAddress.atomicAdd(uint(elemIdx0), sum_v2[0]);
                        }
                    }
                    packed_col_idx += NUM_VECTORS;
                }
            }
            else
            {
                for (int j = 0; j < NumIters; j++)
                {
                    if (packed_col_idx < ColsPacked)
                    {
                        int elemIdx0 = packed_col_idx * 2;
                        int elemIdx1 = elemIdx0 + 1;

                        U sum0 = U(0);
                        U sum1 = U(0);
                        for (int i = 0; i < SubgroupCount; i++)
                        {
                            sum0 = sum0 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded + elemIdx0);
                            if (elemIdx1 < MPadded)
                                sum1 = sum1 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded + elemIdx1);
                        }

                        if (elemIdx0 < M)
                            biasAddress.atomicAdd(uint(elemIdx0), sum0);
                        if (elemIdx1 < M)
                            biasAddress.atomicAdd(uint(elemIdx1), sum1);
                    }
                    packed_col_idx += NUM_VECTORS;
                }
            }
        }
    }

    // =========================================================================
    // Outer product: dW[m][k] = sum_{all threads} dOut_thread[m] * input_thread[k]
    //
    // MatA from FromLocalColumn: thread identity in K (columns of A).
    // MatB from FromLocalRow: thread identity in K (rows of B).
    // MMA sums over K = sums over threads.
    //
    // Result stored in native tiled layout via storeNative + atomicAdd.
    // Cross-warp reduction via shmem storeNative/loadNative.
    // =========================================================================

    static const int NColGroupsOPA = (N + COLUMN_A - 1) / COLUMN_A;
    static const int OPA_CoopMatCRows = (M + ROW_C - 1) / ROW_C;
    static const int OPA_CoopMatCColumns = (K + COLUMN_C - 1) / COLUMN_C;
    static const int OPA_TotalTiles = OPA_CoopMatCRows * OPA_CoopMatCColumns;
    static const int OPA_ElementsPerLane = (ROW_C * COLUMN_C) / N;
    static const int OPA_RegsPerLane = (OPA_ElementsPerLane * sizeof(T) + 3) / 4;
    static const int OPA_TileStride = (OPA_RegsPerLane * N * 4 + sizeof(uint4) - 1) / sizeof(uint4);

    // Phase 1: Compute outer product into matC (passed as inout).
    [require(cuda, subgroup_basic)]
    static void outerProductCompute<TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        InArrayTypeA inputVectorA,
        InArrayTypeB inputVectorB,
        inout MatC matC[OPA_TotalTiles])
            where TypeA : __BuiltinFloatingPointType
            where TypeA.Differential == TypeA
            where TypeB : __BuiltinFloatingPointType
            where TypeB.Differential == TypeB
            where InArrayTypeA : IArrayAccessor<TypeA>
            where InArrayTypeB : IArrayAccessor<TypeB>
    {
        [ForceUnroll]
        for (int k = 0; k < NColGroupsOPA; k++)
        {
            MatA matAArr[OPA_CoopMatCRows];
            [ForceUnroll]
            for (int i = 0; i < OPA_CoopMatCRows; i++)
            {
                half localRowA[COLUMN_A];
                [ForceUnroll]
                for (int m = 0; m < COLUMN_A; m++)
                {
                    int elemIdx = i * ROW_A + m;
                    localRowA[m] = (elemIdx < M) ? __realCast<half>(inputVectorA[elemIdx]) : half(0.0);
                }
                matAArr[i] = MatA.FromLocalRow(localRowA, k);
                matAArr[i].ChangeMajor();
            }

            [ForceUnroll]
            for (int j = 0; j < OPA_CoopMatCColumns; j++)
            {
                half localColB[ROW_B];
                [ForceUnroll]
                for (int n = 0; n < ROW_B; n++)
                {
                    int elemIdx = j * COLUMN_B + n;
                    localColB[n] = (elemIdx < K) ? __realCast<half>(inputVectorB[elemIdx]) : half(0.0);
                }
                var matB = MatB.FromLocalColumn(localColB, k);
                matB.ChangeMajor();

                [ForceUnroll]
                for (int i = 0; i < OPA_CoopMatCRows; i++)
                {
                    int index = i * OPA_CoopMatCColumns + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                                            >(matAArr[i], matB, matC[index]);
                }
            }
        }
    }

    // Phase 2: Cross-warp reduction (same pattern as MMAHelper.sumReduceTilesAllAtOnce).
    [require(cuda, subgroup_basic)]
    static void outerProductReduce<ShMemSize : ISharedMemorySize, int SubgroupCount>(
        inout MatC matC[OPA_TotalTiles],
        int subgroupId,
        uint sharedMemoryOffset)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;

        if (SubgroupCount <= 1)
            return;

        for (int k = 1; k < SubgroupCount; k <<= 1)
        {
            uint subgroupOffset = (subgroupId / (k << 1)) * (OPA_TotalTiles * OPA_TileStride);
            uint base = sharedMemoryOffset + subgroupOffset;

            if (subgroupId % (k << 1) == k)
            {
                [ForceUnroll]
                for (int idx = 0; idx < OPA_TotalTiles; idx++)
                    matC[idx].storeNative(ShMemPool.data, base + idx * OPA_TileStride);
            }

            GroupMemoryBarrierWithGroupSync();

            if (subgroupId % (k << 1) == 0)
            {
                if (subgroupId != (SubgroupCount - 1))
                {
                    [ForceUnroll]
                    for (int idx = 0; idx < OPA_TotalTiles; idx++)
                    {
                        MatC other;
                        other.loadNative(ShMemPool.data, base + idx * OPA_TileStride);
                        matC[idx] = matC[idx] + other;
                    }
                }
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Phase 3: Store matC tiles to global weight buffer via storeNative + atomicAdd.
    // Weight gradient uses native fragment layout (same as weight buffer).
    [require(cuda, subgroup_basic)]
    static void outerProductStore<U, Address, ShMemSize : ISharedMemorySize>(
        inout MatC matC[OPA_TotalTiles],
        uint sharedMemoryOffset,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        const uint laneId = WaveGetLaneIndex();

        // storeNative all tiles to shmem first
        [ForceUnroll]
        for (int idx = 0; idx < OPA_TotalTiles; idx++)
            matC[idx].storeNative(ShMemPool.data, sharedMemoryOffset + idx * OPA_TileStride);

        GroupMemoryBarrierWithWaveSync();

        // atomicAdd from shmem to global in native layout using half2.
        // Each tile has OPA_TileStride uint4 = OPA_TileStride * 4 uint32 = OPA_TileStride * 8 half values.
        // Per thread: OPA_RegsPerLane registers = OPA_RegsPerLane half2 pairs.
        if (sizeof(T) == 2)
        {
            static const int H2PerTile = OPA_TileStride * (sizeof(uint4) / sizeof(U)) / 2;
            static const int TotalH2 = OPA_TotalTiles * H2PerTile;
            static const int ItersPerLane = (TotalH2 + N - 1) / N;

            [ForceUnroll]
            for (uint p = 0; p < ItersPerLane; p++)
            {
                int h2Idx = int(laneId) + int(p) * N;
                if (h2Idx < TotalH2)
                {
                    int tileIdx = h2Idx / H2PerTile;
                    int localH2 = h2Idx % H2PerTile;

                    int i = tileIdx / OPA_CoopMatCColumns;
                    int j = tileIdx % OPA_CoopMatCColumns;

                    // Native layout: 1:1 mapping from shmem to global.
                    // Two MatC tiles (j, j+1) fill one 16×16 weight tile.
                    int weightTileCol = j / 2;
                    int matcLocalOffset = (j % 2) * H2PerTile;

                    int globalOffset = tileElementOffset(i, weightTileCol) / 2 + matcLocalOffset + localH2;

                    vector<U, 2> h2 = ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, h2Idx);
                    let addr = weightAddress.getOffset(globalOffset * 2);
                    addr.atomicAdd(0, h2);
                }
            }
        }

        GroupMemoryBarrierWithWaveSync();
    }

    // Main entry: matC lives here, passed as inout to each phase.
    [ForceInline]
    [require(cuda, subgroup_basic)]
    VISIBILITY_LEVEL static void outerProductAccumulate<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB,
                                                        ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
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

        MatC matC[OPA_TotalTiles];
        [ForceUnroll]
        for (int idx = 0; idx < OPA_TotalTiles; idx++)
            matC[idx].fill(T(0.0f));

        outerProductCompute<TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
            inputVectorA, inputVectorB, matC);

        GroupMemoryBarrierWithGroupSync();

        outerProductReduce<ShMemSize, SubgroupCount>(
            matC, int(subgroupIndex), sharedMemoryOffset);

        if (subgroupIndex == 0)
        {
            outerProductStore<U, Address, ShMemSize>(
                matC, sharedMemoryOffset, weightAddress);
        }

        GroupMemoryBarrierWithGroupSync();
    }

}
