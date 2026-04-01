implementing neural;

#include "common-def.slang"

// MMAHelperNew: Tiled MMA using m16n16k16 with no shared memory at all.
//
// MatA (weight): loaded directly from tiled global memory into CoopMat registers.
//   The weight matrix must be pre-tiled into 16×16 tiles in row-major tile order.
//   M and K must be multiples of 16.
//
// MatB (input): constructed from each thread's local input vector via buildAllTilesFromArray
//   + ChangeMajor + copyFrom. Input is read from a StructuredBuffer per K-tile.
//
// MatC (output): extracted to each thread's local output vector via extractColFromMatC.
//
// MMA shape: m16n16k16 (internally 2x m16n8k16)
//   MatA: 16×16 (half)
//   MatB: 16×16 (half)
//   MatC: 16×16 (T)
//
// For a 32-thread warp with N_tile=16: 2 MatB/MatC tiles cover all 32 threads.
//
// TransposeA:
//   When true, computes W^T × input instead of W × input.
//   LoadRaw loads the tile in its stored (row-major) layout, then ChangeMajor()
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

    // m16n16k16 shape constants (internally 2x m16n8k16)
    static const int ROW_A = 16;
    static const int COLUMN_A = 16;
    static const int ROW_B = 16;
    static const int COLUMN_B = 16;
    static const int ROW_C = 16;
    static const int COLUMN_C = 16;

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
        var matA = MatA.LoadRaw(bit_cast<RWStructuredBuffer<half>>(tileAddr.getBuffer()), tileAddr.getBaseIndex());

        if (TransposeA)
        {
            matA.ChangeMajor();
            // ChangeMajor transposes each 8x8 sub-block via movmatrix, but the
            // full 16x16 transpose also requires swapping the off-diagonal blocks.
            uint r1 = matA.fragmentRead(1);
            uint r2 = matA.fragmentRead(2);
            matA.fragmentWrite(1, r2);
            matA.fragmentWrite(2, r1);
        }

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
            matC[i].clear();

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
                let matB = buildMatBFromLocalCol(localColB, int(j));

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

        OutArrayType outputVector;
        [ForceUnroll]
        for (int i = 0; i < NCoopMatRow; i++)
        {
            for (int j = 0; j < NCoopMatColumn; j++)
            {
                int index = i * NCoopMatColumn + j;

                T localColC[ROW_C];
                extractColFromMatC(matC[index], j, localColC);

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
    // Bias gradient reduction helpers
    // =========================================================================

    // 16 threads per matrix group → 2 groups for 32-thread warp.
    static const int NRowGroups = (N + ROW_B - 1) / ROW_B;

    // Number of output tiles: M elements in chunks of 16.
    static const int NBiasTiles = (M + COLUMN_B - 1) / COLUMN_B;

    // Total MatA tiles = column tiles × thread groups.
    static const int TotalBiasTilesMatA = NBiasTiles * NRowGroups;

    // In-warp shuffle reduction: reduces MatA tiles across threads and writes
    // per-warp partial sums to shared memory. No MMA — uses register adds
    // and 3 shfl_down steps per column tile.
    //
    // MatA register layout (f16, 16x16):
    //   reg[0]=(r0,c0), reg[1]=(r1,c0), reg[2]=(r0,c1), reg[3]=(r1,c1)
    // Step 1: Add tiles across NRowGroups (threads 0-15 + 16-31)
    // Step 2: Add row halves (reg[0]+=reg[1], reg[2]+=reg[3])
    // Step 3: shfl_down across gid groups (stride 4, 8, 16)
    // Step 4: gid=0 threads write result to shmem
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    static void reduceMatATilesToShmem<ShMemSize : ISharedMemorySize>(
        uint sharedMemoryOffset,
        inout MatA allMatA[TotalBiasTilesMatA],
        int subgroupId)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        const uint laneId = WaveGetLaneIndex();
        static const int MPadded = (M + 1) & ~1;
        static const int PairsPerWarp = MPadded / 2;
        static const int PairsPerTile = COLUMN_C / 2;
        WaveMask fullMask = WaveMask(0xFFFFFFFF);

        [ForceUnroll]
        for (int tc = 0; tc < NBiasTiles; tc++)
        {
            [ForceUnroll]
            for (int g = 1; g < NRowGroups; g++)
            {
                [ForceUnroll]
                for (int r = 0; r < 4; r++)
                {
                    uint v0 = allMatA[tc * NRowGroups].fragmentRead(r);
                    uint v1 = allMatA[tc * NRowGroups + g].fragmentRead(r);
                    allMatA[tc * NRowGroups].fragmentWrite(r, bit_cast<uint>(bit_cast<half2>(v0) + bit_cast<half2>(v1)));
                }
            }

            {
                uint r0 = allMatA[tc * NRowGroups].fragmentRead(0);
                uint r1 = allMatA[tc * NRowGroups].fragmentRead(1);
                uint r2 = allMatA[tc * NRowGroups].fragmentRead(2);
                uint r3 = allMatA[tc * NRowGroups].fragmentRead(3);

                half2 sum_lo = bit_cast<half2>(r0) + bit_cast<half2>(r1);
                half2 sum_hi = bit_cast<half2>(r2) + bit_cast<half2>(r3);

                [ForceUnroll]
                for (uint stride = 4; stride <= 16; stride <<= 1)
                {
                    uint shfl_lo = WaveMaskReadLaneAt(fullMask, bit_cast<uint>(sum_lo), int(laneId + stride));
                    uint shfl_hi = WaveMaskReadLaneAt(fullMask, bit_cast<uint>(sum_hi), int(laneId + stride));
                    sum_lo = sum_lo + bit_cast<half2>(shfl_lo);
                    sum_hi = sum_hi + bit_cast<half2>(shfl_hi);
                }

                uint gid = laneId >> 2;
                uint tid = laneId & 3;
                if (gid == 0)
                {
                    int base = tc * PairsPerTile;
                    ShMemPool.smemStore<uint>(sharedMemoryOffset, subgroupId * PairsPerWarp + base + int(tid), bit_cast<uint>(sum_lo));
                    if (base + int(tid) + 4 < PairsPerWarp)
                        ShMemPool.smemStore<uint>(sharedMemoryOffset, subgroupId * PairsPerWarp + base + int(tid) + 4, bit_cast<uint>(sum_hi));
                }
            }
        }
    }

    // Cross-warp reduction: reads per-warp partial sums from shmem,
    // sums across warps, and atomicAdds to the global bias buffer.
    [ForceInline]
    [require(cuda, subgroup_basic)]
    static void crossWarpReduceAndStore<U, Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        const uint laneId = WaveGetLaneIndex();
        static const int MPadded = (M + 1) & ~1;
        static const int PairsPerWarp = MPadded / 2;

        const int NUM_VECTORS = SubgroupCount * N;
        const int PairsPerThread = PairsPerWarp / NUM_VECTORS;
        const int NumIters = (PairsPerWarp % NUM_VECTORS) != 0 ? PairsPerThread + 1 : PairsPerThread;

        int pairIdx = subgroupId * N + int(laneId);

        if (sizeof(U) == 2)
        {
            for (int j = 0; j < NumIters; j++)
            {
                if (pairIdx < PairsPerWarp)
                {
                    vector<U, 2> sum_v2 = vector<U, 2>(U(0), U(0));
                    for (int i = 0; i < SubgroupCount; i++)
                        sum_v2 = sum_v2 + ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, pairIdx + i * PairsPerWarp);

                    int elemIdx0 = pairIdx * 2;
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
                pairIdx += NUM_VECTORS;
            }
        }
        else
        {
            for (int j = 0; j < NumIters; j++)
            {
                if (pairIdx < PairsPerWarp)
                {
                    int elemIdx0 = pairIdx * 2;
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
                pairIdx += NUM_VECTORS;
            }
        }
    }

    // =========================================================================
    // Bias gradient reduction v1 (MMA-based):
    // buildAllTilesFromArray → ChangeMajor → copyFrom → MMA(ones × B) →
    // direct register read → shmem → crossWarpReduceAndStore.
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_basic, subgroup_shuffle)]
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
        static const int PairsPerWarp = MPadded / 2;
        static const int PairsPerTile = COLUMN_C / 2;

        // Build MatA tiles from per-thread dOut, then convert to MatB on demand.
        // buildAllTilesFromArray + ChangeMajor + copyFrom is cheaper than
        // per-tile shuffle construction (batched shuffles vs per-tile shuffles).
        static const int TotalBiasTilesMatA = NBiasTiles * NRowGroups;

        MatA allMatA[TotalBiasTilesMatA];
        if (InArrayType is half[M])
        {
            var arr = bit_cast<half[M]>(dOut);
            buildAllTilesFromArray<M, TotalBiasTilesMatA>(arr, allMatA);
        }
        else
        {
            half dOutLocal[M];
            [ForceUnroll]
            for (int i = 0; i < M; i++)
                dOutLocal[i] = __realCast<half>(dOut[i]);
            buildAllTilesFromArray<M, TotalBiasTilesMatA>(dOutLocal, allMatA);
        }

        MatA matOnes;
        matOnes.fill(half(1));

        [ForceUnroll]
        for (int tc = 0; tc < NBiasTiles; tc++)
        {
            MatC matC;
            matC.clear();

            [ForceUnroll]
            for (int g = 0; g < NRowGroups; g++)
            {
                allMatA[tc * NRowGroups + g].ChangeMajor();
                MatB matB;
                matB.copyFrom(allMatA[tc * NRowGroups + g]);
                matC = linalg.coopMatMulAdd<T, false, half, half, T,
                    MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                    >(matOnes, matB, matC);
            }

            // Since C = ones × B, all rows of matC are identical.
            // Each thread's fragment registers already hold its assigned
            // column values — no shuffles needed.
            // f16: reg[0] = packed(col tid*2, tid*2+1), one read per half2.
            // f32: reg[0] = col tid*2, reg[1] = col tid*2+1, convert+pack.
            static const int MatCRegsLo = (sizeof(T) == 2) ? 2 : 4;
            int dstBase = tc * PairsPerTile;
            int remaining = PairsPerWarp - dstBase;
            int pairsToCopy = (remaining < PairsPerTile) ? remaining : PairsPerTile;
            for (int p = int(laneId); p < pairsToCopy; p += N)
            {
                int regBase = (p < PairsPerTile / 2) ? 0 : MatCRegsLo;
                if (sizeof(T) == 2)
                {
                    ShMemPool.smemStore<uint>(sharedMemoryOffset, subgroupId * PairsPerWarp + dstBase + p, matC.fragmentRead(regBase));
                }
                else
                {
                    uint2 u2 = uint2(matC.fragmentRead(regBase), matC.fragmentRead(regBase + 1));
                    ShMemPool.smemStore<uint2>(sharedMemoryOffset, subgroupId * PairsPerWarp + dstBase + p, u2);
                }
            }
        }

        GroupMemoryBarrierWithGroupSync();
        crossWarpReduceAndStore<U, Address, ShMemSize, SubgroupCount>(sharedMemoryOffset, subgroupId, biasAddress);
    }

    // =========================================================================
    // Bias gradient reduction v2: shuffle-based tree reduction (Tin2 style).
    //
    // Instead of MMA (ones × B), reduces via:
    //   1. buildAllTilesFromArray to get MatA tiles (threads in rows, dOut in cols)
    //   2. Register adds to reduce row tiles and row halves (0 shuffles)
    //   3. shfl_down across gid groups (3 shuffles per column tile)
    //   4. Write result from gid=0 threads to shmem
    //   5. Cross-warp reduction (same as v1)
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_basic, subgroup_shuffle)]
    VISIBILITY_LEVEL static void sumReduceRowsV2<U, Address, InArrayType, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        MatA allMatA[TotalBiasTilesMatA];
        if (InArrayType is half[M])
        {
            var arr = bit_cast<half[M]>(dOut);
            buildAllTilesFromArray<M, TotalBiasTilesMatA>(arr, allMatA);
        }
        else
        {
            half dOutLocal[M];
            [ForceUnroll]
            for (int i = 0; i < M; i++)
                dOutLocal[i] = __realCast<half>(dOut[i]);
            buildAllTilesFromArray<M, TotalBiasTilesMatA>(dOutLocal, allMatA);
        }

        reduceMatATilesToShmem<ShMemSize>(sharedMemoryOffset, allMatA, subgroupId);

        GroupMemoryBarrierWithGroupSync();
        crossWarpReduceAndStore<U, Address, ShMemSize, SubgroupCount>(sharedMemoryOffset, subgroupId, biasAddress);
    }

    // =========================================================================
    // Outer product: dW[m][k] = sum_{all threads} dOut_thread[m] * input_thread[k]
    //
    // MatA from buildAllTilesFromArray: thread identity in K (columns of A).
    // MatB from buildAllTilesFromArray + ChangeMajor + copyFrom: thread identity in K (rows of B).
    // MMA sums over K = sums over threads.
    //
    // Result stored in native tiled layout via StoreRaw + atomicAdd.
    // Cross-warp reduction via shmem StoreRaw/LoadRaw.
    // =========================================================================

    static const int NColGroupsOPA = (N + COLUMN_A - 1) / COLUMN_A;
    static const int OPA_CoopMatCRows = (M + ROW_C - 1) / ROW_C;
    static const int OPA_CoopMatCColumns = (K + COLUMN_C - 1) / COLUMN_C;
    static const int OPA_TotalTiles = OPA_CoopMatCRows * OPA_CoopMatCColumns;
    static const int OPA_ElementsPerLane = (ROW_C * COLUMN_C) / N;
    static const int OPA_RegsPerLane = (OPA_ElementsPerLane * sizeof(T) + 3) / 4;
    static const int OPA_TileStride = (OPA_RegsPerLane * N * 4 + sizeof(uint4) - 1) / sizeof(uint4);

    // Number of 16x16 MatA tiles for the input vector (covering K elements).
    static const int OPA_InputTileCols = (K + COLUMN_A - 1) / COLUMN_A;
    static const int OPA_TotalInputMatA = NColGroupsOPA * OPA_InputTileCols;

    // Number of 16x16 MatA tiles for the dOut vector.
    static const int OPA_TotalDOutMatA = NColGroupsOPA * OPA_CoopMatCRows;

    // Builds all CoopMat fragments from a per-thread input array using optimized
    // warp shuffle. Single flat loop over all packed columns — matches Tin2's
    // Vector::from_array structure exactly. Tile index derived from col_packed.
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void buildAllTilesFromArray<int ArraySize, int OutN>(
        inout half data[ArraySize],
        inout MatA outArray[OutN])
    {
        uint laneid = WaveGetLaneIndex();
        let packedData = bit_cast<uint[ArraySize / 2]>(data);

        static const int TotalColsPacked = ArraySize / 2;
        static const int TileColsPacked = COLUMN_A / 2;
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

                outArray[frag_idx].fragmentWrite(reg_idx, value);
            }
        }
    }

    // Builds one MatB tile from per-thread column data, on demand.
    // Direct Slang implementation of MatB column construction (16x16).
    // matIndex selects which 16-thread group (0 or 1).
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static MatB buildMatBFromLocalCol(
        inout half localCol[ROW_B],
        int matIndex)
    {
        uint laneid = WaveGetLaneIndex();
        let packedCol = bit_cast<uint[ROW_B / 2]>(localCol);

        uint gid = laneid >> 2;
        uint tid = laneid & 3;
        uint base = uint(matIndex) * 16;
        WaveMask fullMask = WaveMask(0xFFFFFFFF);

        uint reg0 = 0, reg1 = 0, reg2 = 0, reg3 = 0;

        [ForceUnroll]
        for (uint t = 0; t < 4; t++)
        {
            uint tmp0 = WaveMaskReadLaneAt(fullMask, packedCol[int(t)],     int(gid + base));
            uint tmp1 = WaveMaskReadLaneAt(fullMask, packedCol[int(t + 4)], int(gid + base));
            uint tmp2 = WaveMaskReadLaneAt(fullMask, packedCol[int(t)],     int(gid + base + 8));
            uint tmp3 = WaveMaskReadLaneAt(fullMask, packedCol[int(t + 4)], int(gid + base + 8));

            if (tid == t)
            {
                reg0 = tmp0;
                reg1 = tmp1;
                reg2 = tmp2;
                reg3 = tmp3;
            }
        }

        MatB result;
        result.fragmentWrite(0, reg0);
        result.fragmentWrite(1, reg1);
        result.fragmentWrite(2, reg2);
        result.fragmentWrite(3, reg3);
        return result;
    }

    // Extracts one column per thread from a MatC (accumulator) tile.
    // Slang implementation of mmaToColMatrixCD for both f16 and f32.
    //
    // MatC f16 (16x16, 4 regs): reg0=packed(C[gid,tid*2],C[gid,tid*2+1]),
    //   reg1=packed(C[gid+8,tid*2],C[gid+8,tid*2+1]), reg2/3 = hi 8 cols.
    // MatC f32 (16x16, 8 regs): reg0=C[gid,tid*2], reg1=C[gid,tid*2+1],
    //   reg2=C[gid+8,tid*2], reg3=C[gid+8,tid*2+1], reg4-7 = hi 8 cols.
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void extractColFromMatC(
        MatC matC,
        int matIndex,
        inout T localCol[ROW_C])
    {
        uint laneid = WaveGetLaneIndex();
        WaveMask fullMask = WaveMask(0xFFFFFFFF);

        if (sizeof(T) == 2)
        {
            // f16 path: 4 regs, each packed pair of halves.
            // 16x16 = lo(regs 0,1) + hi(regs 2,3), matIndex selects 8-col group.
            uint col_in_mat = laneid % 8;
            uint tid_src = col_in_mat / 2;
            uint col_shift = (col_in_mat & 1) * 16;

            int regBase = matIndex * 2;  // 0 for lo, 2 for hi

            [ForceUnroll]
            for (int row = 0; row < 16; row++)
            {
                uint gid_src = (row < 8) ? uint(row) : uint(row - 8);
                int reg_sel = regBase + ((row < 8) ? 0 : 1);
                uint v = WaveMaskReadLaneAt(fullMask, matC.fragmentRead(reg_sel), int(gid_src * 4 + tid_src));
                localCol[row] = __realCast<T>(bit_cast<half>(uint16_t(v >> col_shift)));
            }
        }
        else
        {
            // f32 path: 8 regs. 16x16 = lo(regs 0-3) + hi(regs 4-7).
            // reg layout: reg[col_offset + row_half*2], col_offset=col&1, row_half=row<8?0:1
            uint col_in_mat = laneid % 8;
            uint tid_src = col_in_mat / 2;
            uint col_offset = col_in_mat & 1;
            int regBase = matIndex * 4;  // 0 for lo, 4 for hi
            int src_reg_lo = regBase + int(col_offset);
            int src_reg_hi = regBase + 2 + int(col_offset);

            [ForceUnroll]
            for (int g = 0; g < 8; g++)
            {
                uint src_lane = uint(g) * 4 + tid_src;
                uint val_lo = WaveMaskReadLaneAt(fullMask, matC.fragmentRead(src_reg_lo), int(src_lane));
                uint val_hi = WaveMaskReadLaneAt(fullMask, matC.fragmentRead(src_reg_hi), int(src_lane));
                localCol[g] = __realCast<T>(val_lo);
                localCol[g + 8] = __realCast<T>(val_hi);
            }
        }
    }

    // Per-tile transpose for MatA: swaps register (row,col)→(col,row).
    // Equivalent to Tin2's MMAMatrix::transpose().
    //
    // MatA has 4 registers per thread:
    //   reg 0=(r0,c0), 1=(r1,c0), 2=(r0,c1), 3=(r1,c1)
    // Transpose swaps reg 1↔2.
    VISIBILITY_LEVEL static void transposeTilesA<int Count>(inout MatA tiles[Count])
    {
        [ForceUnroll]
        for (int idx = 0; idx < Count; idx++)
        {
            uint r1 = tiles[idx].fragmentRead(1);
            uint r2 = tiles[idx].fragmentRead(2);
            tiles[idx].fragmentWrite(1, r2);
            tiles[idx].fragmentWrite(2, r1);
        }
    }

    // Per-tile transpose for MatC (accumulator): swaps register (row,col)→(col,row).
    // Equivalent to Tin2's w.transpose() on the MMA result.
    //
    // MatC has 4 registers (two m16n8k16 accumulators, 2 regs each):
    //   reg 0=(r0,c0), 1=(r1,c0), 2=(r0,c1), 3=(r1,c1)
    // Transpose swaps reg 1↔2.
    static void transposeTilesC<int Count>(inout MatC tiles[Count])
    {
        [ForceUnroll]
        for (int idx = 0; idx < Count; idx++)
        {
            uint r1 = tiles[idx].fragmentRead(1);
            uint r2 = tiles[idx].fragmentRead(2);
            tiles[idx].fragmentWrite(1, r2);
            tiles[idx].fragmentWrite(2, r1);
        }
    }

    // Outer product compute: dW[m][k] = sum_{all threads} dOut[m] * input[k]
    [ForceInline]
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
        // --- Build dOut as 16x16 MatA tiles into allDOut ---
        // After buildAllTilesFromArray, tiles represent a (WarpSize × M) matrix
        // (thread index in rows, dOut elements in columns).
        // transposeTileArray transposes to (M × WarpSize), matching Tin2's b.transpose(),
        // so that dOut element indices map to MMA rows (M dimension).
        MatA allDOut[OPA_TotalDOutMatA];
        if (InArrayTypeA is half[M])
        {
            var arrA = bit_cast<half[M]>(inputVectorA);
            buildAllTilesFromArray<M, OPA_TotalDOutMatA>(arrA, allDOut);
        }
        else
        {
            half localA[M];
            [ForceUnroll]
            for (int m = 0; m < M; m++)
                localA[m] = __realCast<half>(inputVectorA[m]);
            buildAllTilesFromArray<M, OPA_TotalDOutMatA>(localA, allDOut);
        }

        transposeTilesA<OPA_TotalDOutMatA>(allDOut);

        [ForceUnroll]
        for (int i = 0; i < OPA_TotalDOutMatA; i++)
            allDOut[i].ChangeMajor();

        // --- Build input as MatA tiles, then ChangeMajor + copyFrom to MatB ---
        MatA allInputA[OPA_TotalInputMatA];
        if (InArrayTypeB is half[K])
        {
            var arrB = bit_cast<half[K]>(inputVectorB);
            buildAllTilesFromArray<K, OPA_TotalInputMatA>(arrB, allInputA);
        }
        else
        {
            half localB[K];
            [ForceUnroll]
            for (int n = 0; n < K; n++)
                localB[n] = __realCast<half>(inputVectorB[n]);
            buildAllTilesFromArray<K, OPA_TotalInputMatA>(localB, allInputA);
        }

        MatB allInputB[OPA_TotalInputMatA];
        [ForceUnroll]
        for (int i = 0; i < OPA_TotalInputMatA; i++)
        {
            allInputA[i].ChangeMajor();
            allInputB[i].copyFrom(allInputA[i]);
        }

        // --- MMA loop using m16n16k16 ---
        [ForceUnroll]
        for (int k = 0; k < NColGroupsOPA; k++)
        {
            [ForceUnroll]
            for (int j16 = 0; j16 < OPA_InputTileCols; j16++)
            {
                [ForceUnroll]
                for (int i = 0; i < OPA_CoopMatCRows; i++)
                {
                    matC[i * OPA_CoopMatCColumns + j16] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                                            >(allDOut[i * 2 + k], allInputB[j16 * 2 + k],
                                              matC[i * OPA_CoopMatCColumns + j16]);
                }
            }
        }
    }

    // Phase 2: Cross-warp reduction (same pattern as MMAHelper.sumReduceTilesAllAtOnce).
    [ForceInline]
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
                    matC[idx].StoreRaw(ShMemPool.data, base + idx * OPA_TileStride);
            }

            GroupMemoryBarrierWithGroupSync();

            if (subgroupId % (k << 1) == 0)
            {
                if (subgroupId != (SubgroupCount - 1))
                {
                    [ForceUnroll]
                    for (int idx = 0; idx < OPA_TotalTiles; idx++)
                    {
                        let other = MatC.LoadRaw(ShMemPool.data, base + idx * OPA_TileStride);
                        matC[idx] = matC[idx] + other;
                    }
                }
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Phase 3: Store matC tiles to global weight buffer via StoreRaw + atomicAdd.
    // Weight gradient uses native fragment layout (same as weight buffer).
    //
    // Iterates linearly over all half2 elements, using a single base address
    // and incrementing offset to avoid per-element getOffset calls.
    [ForceInline]
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

        // StoreRaw all tiles to shmem first
        [ForceUnroll]
        for (int idx = 0; idx < OPA_TotalTiles; idx++)
            matC[idx].StoreRaw(ShMemPool.data, sharedMemoryOffset + idx * OPA_TileStride);

        GroupMemoryBarrierWithWaveSync();

        if (sizeof(T) == 2)
        {
            static const int H2PerTile = OPA_TileStride * (sizeof(uint4) / sizeof(U)) / 2;
            static const int TotalH2 = OPA_TotalTiles * H2PerTile;
            static const int ItersPerLane = (TotalH2 + N - 1) / N;

            // The shmem native layout is a 1:1 mapping to the global tiled weight layout:
            // matC tiles are stored sequentially, and the weight buffer uses the same
            // native fragment ordering. So globalOffset = h2Idx (no remapping needed).
            [ForceUnroll]
            for (uint p = 0; p < ItersPerLane; p++)
            {
                int h2Idx = int(laneId) + int(p) * N;
                if (h2Idx < TotalH2)
                {
                    vector<U, 2> h2 = ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, h2Idx);
                    weightAddress.atomicAdd(uint(h2Idx * 2), h2);
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
            matC[idx].clear();

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
