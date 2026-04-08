implementing neural;


// MMAHelperNewV2: Same logic as V2 but using WaveMatrix types for readability.
//
// Key type mappings:
//   V2: MatA tiles[N]        → V3: MatrixA<Rows, Cols> (a single WaveMatrix)
//   V2: MatB tiles[N]        → V3: MatrixB<Rows, Cols>
//   V2: MatC tiles[N]        → V3: MatrixC<T, Rows, Cols>
//
// All tile indexing, MMA loops, shmem store/load, and cross-warp reductions
// are delegated to WaveMatrix methods.

[require(cuda)]
internal struct MMAHelperNewV2<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int M = OutputSize;
    static const int K = InputSize;
    static const int N = SubgroupSize;  // WarpSize = 32

    static const int ROW = 16;
    static const int COL = 16;

    // WaveMatrix types for the various roles
    internal typealias DOutMatrix   = MatrixA<N, M>;
    internal typealias InputMatrix  = MatrixA<N, K>;

    internal typealias ResultMatrix = MatrixC<T, N, K>;
    internal typealias WeightMatrix = MatrixB<M, K>;
    internal typealias OPResult     = MatrixC<T, M, K>;
    static const int OPResultTileStride = OPResult.TileStride;

    internal static const int DOutTileCount   = DOutMatrix.TileCount;
    internal static const int InputTileCount  = InputMatrix.TileCount;
    internal static const int ResultTileCount = ResultMatrix.TileCount;
    internal static const int OP_TotalTiles   = OPResult.TileCount;

    // =========================================================================
    // from_array / to_array: delegated to WaveMatrix methods
    // =========================================================================

    [require(cuda, subgroup_shuffle)]
    internal static void fromArrayPacked<int PackedSize>(
        uint packedData[PackedSize],
        inout DOutMatrix mat)
    {
        mat.fromArrayPacked<PackedSize>(packedData);
    }

    [require(cuda, subgroup_shuffle)]
    internal static void fromArrayPackedInput<int PackedSize>(
        uint packedData[PackedSize],
        inout InputMatrix mat)
    {
        mat.fromArrayPacked<PackedSize>(packedData);
    }

    [require(cuda, subgroup_shuffle)]
    internal static void toArrayPacked<T, int PackedSize>(
        MatrixC<T, N, K> mat,
        inout uint packedOut[PackedSize])
            where T : __BuiltinFloatingPointType
            where T.Differential == T
    {
        mat.toArrayPacked<PackedSize>(packedOut);
    }

    // =========================================================================
    // High-level forward/backward API
    //
    // Takes WaveTangledVector inputs directly, handles packing/unpacking.
    // =========================================================================

    internal static const int PackedSizeInput = K / (sizeof(uint) / sizeof(T));
    internal static const int PackedSizeOutput = M / (sizeof(uint) / sizeof(T));

    // backward: given input and dOut, compute dInput, accumulate dWeight and dBias
    //
    // Takes WaveTangledVector inputs directly, handles packing/unpacking internally.
    // Performs all three backward operations:
    //   1. dInput = W^T × dOut (transpose MMA)
    //   2. dBias += sum(dOut) (bias reduce)
    //   3. dWeight += dOut ⊗ input (outer product accumulate)
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static WaveTangledVector<T, ShMemSize, K, N, SubgroupCount>
    backward<Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        WaveTangledVector<T, ShMemSize, K, N, SubgroupCount> input,
        WaveTangledVector<T, ShMemSize, M, N, SubgroupCount> dOut,
        Address weightAddress,
        Address weightGradAddress,
        Address biasGradAddress)
        where Address : IPointerLikeAddress<T>
    {
        let dOutPacked = bit_cast<uint[PackedSizeOutput]>(dOut.data);
        let inputPacked = bit_cast<uint[PackedSizeInput]>(input.data);

        DOutMatrix dOutMat;
        dOutMat.fromArrayPacked<PackedSizeOutput>(dOutPacked);

        InputMatrix inputMat;
        inputMat.fromArrayPacked<PackedSizeInput>(inputPacked);

        // 1. Transpose MMA: dIn = W^T * dOut
        ResultMatrix resultMat;
        mma<true, Address>(dOutMat, weightAddress, resultMat);

        // 2. Bias reduce
        int subgroupId = int(getWaveId());
        biasReduce<T, Address, ShMemSize, SubgroupCount>(
            0u, dOutMat, subgroupId, biasGradAddress);

        // 3. Outer product: accumulate dWeight
        outerProduct<T, Address, ShMemSize, SubgroupCount>(
            0u, dOutMat, inputMat, weightGradAddress);

        // 4. Extract dInput from result
        uint resultPacked[PackedSizeInput];
        resultMat.toArrayPacked<PackedSizeInput>(resultPacked);

        typealias InputVec = WaveTangledVector<T, ShMemSize, K, N, SubgroupCount>;
        InputVec dInput;
        dInput.data = bit_cast<T[InputVec.Uint4AlignedInputSize]>(resultPacked);
        return dInput;
    }

    // =========================================================================
    // Transpose MMA: dIn = W^T × dOut
    //
    // Tin2 flow:
    //   w: MatB(M×K) → w.transpose(): swap reg 1↔2 + tile grid swap
    //   → changeMajorAxis: MatB(M×K)
    //   → mul(dOut: MatA(32×M), w_t: MatB(M×K)) → MatA(32×K)
    // =========================================================================

    static const int TransposeMMA_BTileRows = (M + ROW - 1) / ROW;
    static const int TransposeMMA_BTileCols = (K + COL - 1) / COL;

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static void mma<bool Transpose, Address>(
        DOutMatrix dOut,
        Address weightAddress,
        inout ResultMatrix result)
            where Address : IPointerLikeAddress<T>
    {
        // Load weights as MatB(M×K)
        WeightMatrix w;
        w.loadWeightRaw<T, Address>(weightAddress);

        if (Transpose)
        {
            // Backward: W^T × dOut
            // Transpose: MatB(M×K) → MatA(M×K) via copyFrom + sub-tile transpose
            MatrixA<M, K> wA;
            [ForceUnroll]
            for (int i = 0; i < w.TileCount; i++)
                wA.tiles[i].copyFrom(w.tiles[i]);
            wA.transposeSubMatrix();

            // changeMajorAxis: MatA(M×K) → MatB(M×K)
            let wT = toMatrixB<M, K>(wA);

            // MMA: result(32×K) = dOut(32×M) × wT(M×K)
            result = matMul<T, N, M, K>(dOut, wT);
        }
        else
        {
            // Forward: input × W
            // Weight is already MatB(M×K), use directly
            // MMA: result(32×M) = input(32×K) × W(K×M)
            // Note: for forward, DOutMatrix is actually InputMatrix (32×K)
            // and result is (32×M), but the types must match the template params.
            result = matMul<T, N, M, K>(dOut, w);
        }
    }

    // =========================================================================
    // Outer product: dW = dOut ⊗ input (summed across threads via MMA)
    //
    // Tin2 flow:
    //   at = change_major_axis(input):     MatA(32×K) → MatB(32×K)
    //   bt = transpose(dOut) + CMA:        MatA(32×M) → MatA(M×32)
    //   w  = mul(bt, at):                  MatA(M×32) × MatB(32×K) → MatA(M×K)
    //
    // Cross-warp reduction via shmem, then atomicAdd to global.
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static void outerProduct<U, Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        inout DOutMatrix dOut,
        inout InputMatrix input,
        Address weightAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        const uint subgroupIndex = getWaveId();

        // changeMajorAxis(input): MatA(32×K) → per-tile ChangeMajor in-place
        input.changeMajor();

        // dOut transpose + changeMajorAxis: sub-tile transpose then ChangeMajor
        dOut.transposeSubMatrix();
        dOut.changeMajor();

        // Convert input tiles to MatB via copyFrom
        MatrixB<N, K> inputB;
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputB.tiles[i].copyFrom(input.tiles[i]);

        // Convert dOut tiles to MatA with swapped grid (M×32 layout)
        // dOut was (32×M), after transposeSubMatrix it's logically (M×32)
        // but tile indices need remapping: src col-major(32×M) → dst col-major(M×32)
        static const int OP_ATileRows = (M + ROW - 1) / ROW;
        static const int OP_ATileCols = (N + COL - 1) / COL;  // 32/16 = 2
        MatrixA<M, N> dOutT;
        [ForceUnroll]
        for (int c = 0; c < OP_ATileCols; c++)
        {
            [ForceUnroll]
            for (int r = 0; r < OP_ATileRows; r++)
            {
                // dOut tiles are col-major with NRowTiles rows × (M/16) cols
                // After transpose they become (M/16) rows × NRowTiles cols
                int srcIdx = r * DOutMatrix.TileRows + c;  // swapped
                int dstIdx = c * OP_ATileRows + r;
                dOutT.tiles[dstIdx] = dOut.tiles[srcIdx];
            }
        }

        // MMA: C(M×K) = A(M×32) × B(32×K)
        OPResult matC = matMul<T, M, N, K>(dOutT, inputB);

        // Cross-warp reduction
        if (SubgroupCount > 1)
        {
            for (int s = 1; s < SubgroupCount; s <<= 1)
            {
                uint subgroupOffset = (subgroupIndex / (s << 1)) * (OP_TotalTiles * OPResultTileStride);
                uint base = sharedMemoryOffset + subgroupOffset;

                if (subgroupIndex % (s << 1) == uint(s))
                    matC.storeRaw<ShMemSize>(base);

                GroupMemoryBarrierWithGroupSync();

                if (subgroupIndex % (s << 1) == 0)
                {
                    if (subgroupIndex != uint(SubgroupCount - 1))
                    {
                        OPResult other;
                        other.loadRaw<ShMemSize>(base);
                        matC = matC.add(other);
                    }
                }

                GroupMemoryBarrierWithGroupSync();
            }
        }

        // Store to global via shmem + atomicAdd
        if (subgroupIndex == 0)
        {
            typealias ShMemPool = SharedMemoryPool<ShMemSize>;
            const uint laneId = WaveGetLaneIndex();

            matC.storeRaw<ShMemSize>(sharedMemoryOffset);
            GroupMemoryBarrierWithWaveSync();

            if (sizeof(T) == 2)
            {
                static const int H2PerTile = OPResultTileStride * (sizeof(uint4) / sizeof(U)) / 2;
                static const int TotalH2 = OP_TotalTiles * H2PerTile;
                static const int ItersPerLane = (TotalH2 + N - 1) / N;

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
        }

        GroupMemoryBarrierWithGroupSync();
    }

    // =========================================================================
    // Bias reduce: dBias = sum(dOut) across threads
    // Same logic as V2.
    // =========================================================================

    static const int NBiasTiles = (M + COL - 1) / COL;
    static const int NRowTiles = (N + ROW - 1) / ROW;
    static const int NRowGroups = NRowTiles;
    internal static const int TotalBiasTilesMatA = NBiasTiles * NRowGroups;

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static void biasReduce<U, Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        DOutMatrix dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        const uint laneId = WaveGetLaneIndex();
        static const int MPadded = (M + 1) & ~1;
        static const int PairsPerWarp = MPadded / 2;
        static const int PairsPerTile = COL / 2;
        WaveMask fullMask = WaveMask(0xFFFFFFFF);

        // Work on a local copy
        DOutMatrix localTiles = dOut;

        [ForceUnroll]
        for (int tc = 0; tc < NBiasTiles; tc++)
        {
            [ForceUnroll]
            for (int g = 1; g < NRowGroups; g++)
            {
                [ForceUnroll]
                for (int r = 0; r < 4; r++)
                {
                    uint v0 = localTiles.tiles[tc * NRowGroups].fragmentRead(r);
                    uint v1 = localTiles.tiles[tc * NRowGroups + g].fragmentRead(r);
                    localTiles.tiles[tc * NRowGroups].fragmentWrite(r, bit_cast<uint>(bit_cast<half2>(v0) + bit_cast<half2>(v1)));
                }
            }

            {
                uint r0 = localTiles.tiles[tc * NRowGroups].fragmentRead(0);
                uint r1 = localTiles.tiles[tc * NRowGroups].fragmentRead(1);
                uint r2 = localTiles.tiles[tc * NRowGroups].fragmentRead(2);
                uint r3 = localTiles.tiles[tc * NRowGroups].fragmentRead(3);

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

        GroupMemoryBarrierWithGroupSync();

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
                    U sum0 = U(0);
                    U sum1 = U(0);
                    for (int i = 0; i < SubgroupCount; i++)
                    {
                        static const int MPadded2 = (M + 1) & ~1;
                        sum0 = sum0 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded2 + pairIdx * 2);
                        sum1 = sum1 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded2 + pairIdx * 2 + 1);
                    }

                    int elemIdx0 = pairIdx * 2;
                    int elemIdx1 = elemIdx0 + 1;

                    if (elemIdx0 < M)
                        biasAddress.atomicAdd(uint(elemIdx0), sum0);
                    if (elemIdx1 < M)
                        biasAddress.atomicAdd(uint(elemIdx1), sum1);
                }
                pairIdx += NUM_VECTORS;
            }
        }

        GroupMemoryBarrierWithGroupSync();
    }
}
