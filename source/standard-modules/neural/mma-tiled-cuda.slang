implementing neural;


// TiledMMACuda: CUDA backend for tiled MMA using WaveMatrix register ops.
//
// Uses warp shuffles (fromArrayPacked), raw pointer loads (loadWeightRaw),
// and register-level transforms (changeMajor, transposeSubMatrix) for
// zero-shmem vector↔CoopMat and weight transpose operations.
//
// InputSize and OutputSize must be tile-aligned (multiples of 16).
// Callers with arbitrary sizes should pad via PadToTile before instantiation.
//
// Called by TiledMMAHelper.backward() on CUDA targets.

[require(cuda)]
internal struct TiledMMACuda<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int M = OutputSize;
    static const int K = InputSize;
    static const int N = SubgroupSize;  // WarpSize = 32

    static const int ROW = 16;
    static const int COL = 16;

    // WaveMatrix types for the various roles.
    // Weight is MatB(K, M) matching Tin2's HMatrixB<Z0=InputSize, Z1=OutputSize>.
    internal typealias InputMatrix  = MatrixA<N, K>;
    internal typealias DOutMatrix   = MatrixA<N, M>;
    internal typealias WeightMatrix = MatrixB<K, M>;
    internal typealias ResultMatrix = MatrixC<T, N, M>;   // forward result
    internal typealias BwdResultMatrix = MatrixC<T, N, K>; // backward result (dInput)
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

    // =========================================================================
    // Forward MMA: uses internal mma<false> (WaveMatrix register path)
    // Weight loaded as MatB via loadWeightRaw, input packed as MatA via shuffles.
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>
    forward<Address, ShMemSize : ISharedMemorySize, int SubgroupCount, bool Bias = false>(
        WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
    {
        typealias OutputVec = WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>;
        typealias InputVecT = WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount>;

        // Packed sizes derived from Uint4AlignedInputSize so fromArrayPacked/toArrayPacked
        // always receive arrays aligned to RegColsPacked (4 uints).
        static const int AlignedPackedInputHalf = InputVecT.Uint4AlignedInputSize / 2;
        static const int AlignedPackedResult = OutputVec.Uint4AlignedInputSize * sizeof(T) / sizeof(uint);

        InputMatrix inputMat;
        if (sizeof(T) == 2)
        {
            let inputPacked = bit_cast<uint[AlignedPackedInputHalf]>(input.data);
            inputMat.fromArrayPacked<AlignedPackedInputHalf>(inputPacked);
        }
        else
        {
            half inputHalf[InputVecT.Uint4AlignedInputSize];
            for (int i = 0; i < K; i++)
                inputHalf[i] = __realCast<half>(input[i]);
            for (int i = K; i < InputVecT.Uint4AlignedInputSize; i++)
                inputHalf[i] = half(0);
            let inputPacked = bit_cast<uint[AlignedPackedInputHalf]>(inputHalf);
            inputMat.fromArrayPacked<AlignedPackedInputHalf>(inputPacked);
        }

        // MMA: result(N, M) = input(N, K) × W(K, M)
        ResultMatrix resultMat;
        mma<Address>(inputMat, weightAddress, resultMat);

        // Extract result
        uint resultPacked[AlignedPackedResult];
        resultMat.toArrayPacked<AlignedPackedResult>(resultPacked);

        OutputVec output;
        output.data = bit_cast<T[OutputVec.Uint4AlignedInputSize]>(resultPacked);

        if (Bias)
        {
            for (int i = 0; i < M; i++)
                output.data[i] = output.data[i] + biasAddress.value[uint(i)];
        }

        return output;
    }

    // backward: given input and dOut, compute dInput, accumulate dWeight and dBias
    //
    // Takes WaveTangledVector inputs directly, handles packing/unpacking internally.
    // Performs all three backward operations:
    //   1. dInput = W^T × dOut (transpose MMA)
    //   2. dBias += sum(dOut) (bias reduce)
    //   3. dWeight += dOut ⊗ input (outer product accumulate)
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static WaveTangledVector<T.Differential, ShMemSize, K, N, SubgroupCount>
    backward<Address, ShMemSize : ISharedMemorySize, int SubgroupCount, bool Bias = true>(
        WaveTangledVector<T, ShMemSize, K, N, SubgroupCount> input,
        WaveTangledVector<T, ShMemSize, M, N, SubgroupCount> dOut,
        Address weightAddress,
        Address weightGradAddress,
        Address biasGradAddress)
        where Address : IPointerLikeAddress<T>
    {
        typealias OutputVec = WaveTangledVector<T, ShMemSize, M, N, SubgroupCount>;
        typealias InputVecT = WaveTangledVector<T, ShMemSize, K, N, SubgroupCount>;
        static const int AlignedPackedDOutHalf = OutputVec.Uint4AlignedInputSize / 2;
        static const int AlignedPackedInputHalf = InputVecT.Uint4AlignedInputSize / 2;

        // DOut is loaded as Row vector for MatA (half-typed).
        DOutMatrix dOutMat;
        if (sizeof(T) == 2)
        {
            let dOutPacked = bit_cast<uint[AlignedPackedDOutHalf]>(dOut.data);
            dOutMat.fromArrayPacked<AlignedPackedDOutHalf>(dOutPacked);
        }
        else
        {
            half dOutHalf[OutputVec.Uint4AlignedInputSize];
            for (int i = 0; i < M; i++)
                dOutHalf[i] = __realCast<half>(dOut[i]);
            for (int i = M; i < OutputVec.Uint4AlignedInputSize; i++)
                dOutHalf[i] = half(0);
            let dOutPacked = bit_cast<uint[AlignedPackedDOutHalf]>(dOutHalf);
            dOutMat.fromArrayPacked<AlignedPackedDOutHalf>(dOutPacked);
        }

        // Input is loaded as Row vector for MatA (half-typed).
        InputMatrix inputMat;
        if (sizeof(T) == 2)
        {
            let inputPacked = bit_cast<uint[AlignedPackedInputHalf]>(input.data);
            inputMat.fromArrayPacked<AlignedPackedInputHalf>(inputPacked);
        }
        else
        {
            half inputHalf[InputVecT.Uint4AlignedInputSize];
            for (int i = 0; i < K; i++)
                inputHalf[i] = __realCast<half>(input[i]);
            for (int i = K; i < InputVecT.Uint4AlignedInputSize; i++)
                inputHalf[i] = half(0);
            let inputPacked = bit_cast<uint[AlignedPackedInputHalf]>(inputHalf);
            inputMat.fromArrayPacked<AlignedPackedInputHalf>(inputPacked);
        }

        // 1. Transpose MMA: dIn = dOut × W^T
        BwdResultMatrix resultMat;
        mmaTranspose<Address>(dOutMat, weightAddress, resultMat);

        // 2. Bias reduce
        int subgroupId = int(getWaveId());
        if (Bias)
            biasReduce<T, Address, ShMemSize, SubgroupCount>(
                0u, dOutMat, subgroupId, biasGradAddress);

        // 3. Outer product: accumulate dWeight
        outerProduct<T, Address, ShMemSize, SubgroupCount>(
            0u, dOutMat, inputMat, weightGradAddress);

        // 4. Extract dInput from result (MatC is T-typed, so use T-sized packing)
        typealias InputVec = WaveTangledVector<T.Differential, ShMemSize, K, N, SubgroupCount>;
        static const int AlignedPackedResultK = InputVecT.Uint4AlignedInputSize * sizeof(T) / sizeof(uint);
        uint resultPacked[AlignedPackedResultK];
        resultMat.toArrayPacked<AlignedPackedResultK>(resultPacked);

        InputVec dInput;
        dInput.data = bit_cast<T.Differential[InputVec.Uint4AlignedInputSize]>(resultPacked);
        return dInput;
    }

    // =========================================================================
    // Forward MMA: output = input × W
    //
    // Weight: MatB(K, M) loaded via loadWeightRaw from native tiled layout.
    // Input: MatA(N, K) packed via fromArrayPacked.
    // Result: MatC(N, M) → M=OutputSize elements per lane.
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static void mma<Address>(
        InputMatrix inputMat,
        Address weightAddress,
        inout ResultMatrix result)
            where Address : IPointerLikeAddress<T>
    {
        WeightMatrix w;
        w.loadWeightRaw<T, Address>(weightAddress);
        result = matMul<T, N, K, M>(inputMat, w);
    }

    // =========================================================================
    // Transpose MMA: dIn = dOut × W^T
    //
    // Weight: MatB(K, M) loaded via loadWeightRaw.
    // Transpose: MatB(K,M) → sub-tile transpose + changeMajorAxis → MatB(M,K) with W^T content.
    // dOut: MatA(N, M).
    // Result: MatC(N, K) = dOut(N,M) × W^T(M,K) → K=InputSize elements per lane.
    // =========================================================================

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    internal static void mmaTranspose<Address>(
        DOutMatrix dOut,
        Address weightAddress,
        inout BwdResultMatrix result)
            where Address : IPointerLikeAddress<T>
    {
        // Load weights as MatB(K, M)
        WeightMatrix w;
        w.loadWeightRaw<T, Address>(weightAddress);

        // Transpose: MatB(K,M) → MatB(M,K) via sub-tile transpose + ChangeMajor + tile grid swap
        let wT = w.transpose();

        // MMA: result(N, K) = dOut(N, M) × W^T(M, K)
        result = matMul<T, N, M, K>(dOut, wT);
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
                    if (subgroupIndex + uint(s) < uint(SubgroupCount))
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
            else if (sizeof(U) == 4)
            {
                static const int FloatsPerTile = OPResultTileStride * (sizeof(uint4) / sizeof(U));
                static const int TotalFloats = OP_TotalTiles * FloatsPerTile;
                static const int ItersPerLane = (TotalFloats + N - 1) / N;

                [ForceUnroll]
                for (uint p = 0; p < ItersPerLane; p++)
                {
                    int fIdx = int(laneId) + int(p) * N;
                    if (fIdx < TotalFloats)
                    {
                        U val = ShMemPool.smemLoad<U>(sharedMemoryOffset, fIdx);
                        weightAddress.atomicAdd(uint(fIdx), val);
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
                    uint v0 = localTiles.tiles[tc * NRowGroups].__fragmentRead(r);
                    uint v1 = localTiles.tiles[tc * NRowGroups + g].__fragmentRead(r);
                    localTiles.tiles[tc * NRowGroups].__fragmentWrite(r, bit_cast<uint>(bit_cast<half2>(v0) + bit_cast<half2>(v1)));
                }
            }

            {
                uint r0 = localTiles.tiles[tc * NRowGroups].__fragmentRead(0);
                uint r1 = localTiles.tiles[tc * NRowGroups].__fragmentRead(1);
                uint r2 = localTiles.tiles[tc * NRowGroups].__fragmentRead(2);
                uint r3 = localTiles.tiles[tc * NRowGroups].__fragmentRead(3);

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

        for (int j = 0; j < NumIters; j++)
        {
            if (pairIdx < PairsPerWarp)
            {
                vector<half, 2> sum_h2 = vector<half, 2>(half(0), half(0));
                for (int i = 0; i < SubgroupCount; i++)
                    sum_h2 = sum_h2 + ShMemPool.smemLoad<vector<half, 2>>(sharedMemoryOffset, pairIdx + i * PairsPerWarp);

                int elemIdx0 = pairIdx * 2;
                int elemIdx1 = elemIdx0 + 1;

                if (sizeof(U) == 2)
                {
                    // M is always tile-aligned (multiple of 16), so elemIdx1 < M
                    // is sufficient — no odd-M tail handling needed.
                    if (elemIdx1 < M)
                    {
                        let addr = biasAddress.getOffset(elemIdx0);
                        addr.atomicAdd(0u, vector<U, 2>(__realCast<U>(sum_h2.x), __realCast<U>(sum_h2.y)));
                    }
                }
                else
                {
                    if (elemIdx0 < M)
                        biasAddress.atomicAdd(uint(elemIdx0), __realCast<U>(sum_h2.x));
                    if (elemIdx1 < M)
                        biasAddress.atomicAdd(uint(elemIdx1), __realCast<U>(sum_h2.y));
                }
            }
            pairIdx += NUM_VECTORS;
        }

        GroupMemoryBarrierWithGroupSync();
    }
}
