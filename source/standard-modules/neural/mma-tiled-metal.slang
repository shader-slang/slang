// TiledMMAMetal: Metal backend for tiled MMA using 8x8 simdgroup_matrix
// tiles (matching Metal's hardware MMA size).
//
// Weight memory layout
// --------------------
// Despite the `Tiled*` family name, this Metal backend expects the weight
// matrix W to be a plain M × K **row-major** buffer in memory (stride = K),
// NOT the tile-grid layout used by `TiledMMACuda` / `TiledMMAVulkan`.
// A-tiles are issued as `simdgroup_matrix_8x8_load` with
// `extents=[K, 8], strides=[1, K]`, i.e. the standard wide-row pattern.
//
// The earlier perf investigation on Apple M4 showed that
// the canonical tile-grid pattern (`extents=[8, 8], strides=[1, 8]`)
// defeats the GPU's prefetch reuse after each per-k threadgroup-memory
// write, costing 8–14% at half and 1–6% at float. A re-measurement after
// the ForceUnroll MMA + barrier-after-load + packed-uint4 shmem-write
// changes showed the half gap has mostly collapsed (0–3%), but float
// still pays 1–16% for the tile-grid layout at large sizes, so row-major
// remains the default for this backend.
//
// Callers that pack weights for mixed CUDA/Vulkan/Metal deployment should
// therefore keep a separate row-major packing for the Metal path (or
// equivalently, transpose from tile-grid to row-major on upload).
implementing neural;


static const int MetalTile = 8;

internal typealias MetalMatrixA<T: __BuiltinFloatingPointType, int Rows, int Cols> = WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixA, Rows, Cols>;
internal typealias MetalMatrixB<T: __BuiltinFloatingPointType, int Rows, int Cols> = WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixB, Rows, Cols>;
internal typealias MetalMatrixC<T: __BuiltinFloatingPointType, int Rows, int Cols> = WaveMatrix<T, linalg.CoopMatMatrixUse.MatrixAccumulator, Rows, Cols>;


[require(cooperative_matrix)]
internal struct TiledMMAMetal<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemPool = SharedMemoryPool<ShMemSize>;
    typealias BaseMMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;
    typealias CMShape = BaseMMA.CMShape;

    static const int M = OutputSize;
    static const int K = InputSize;
    static const int N = SubgroupSize;

    static const int NTileRows = (M + MetalTile - 1) / MetalTile;
    static const int NTileCols = (K + MetalTile - 1) / MetalTile;
    static const int TileSizeInElements = MetalTile * MetalTile;

    [ForceInline]
    static int tileElementOffset(int tileRow, int tileCol)
    {
        return (tileRow * NTileCols + tileCol) * TileSizeInElements;
    }

    // =========================================================================
    // Forward: result = W(M,K) × input(K,N)
    //
    // Blocked outer-product form (K outermost, M blocked with M_BLOCK=2).
    // Keeps ~13 simdgroup matrices live to avoid register spills on Apple Silicon.
    //
    // Metal: no hybrid precision -- A, B, C all use the same type T.
    // Uses WaveMatrix methods: loadWeightTiled, fromVectorViaShMem, matMad.
    // =========================================================================

    static const int M_BLOCK = 2;

    // Struct-scope shared state for forward / mmaTranspose. Both functions
    // reuse the same K-loop via `mmaAccumulateMgGroup`, and both rely on the
    // same per-warp shmem layout and 8x8 CoopMat types. Keeping these at
    // struct scope (rather than duplicated as function-local constants) means
    // the helper(s) can access them directly.
    static const int TILES_N_ = N / MetalTile;
    static const int ElemsPerUint4_ = sizeof(uint4) / sizeof(T);
    static const int TileWidthUint4_ = (MetalTile + ElemsPerUint4_ - 1) / ElemsPerUint4_;
    static const int ShmemPerWarpUint4_ = 32 * TileWidthUint4_;

    typealias AMatrix = linalg.CoopMat<T, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixA>;
    typealias BMatrix = linalg.CoopMat<T, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixB>;
    typealias CMatrix = linalg.CoopMat<T, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    // =========================================================================
    // packInputTileToShmem
    //
    // Pack a tile-wide (MetalTile = 8) slice of the input WTV into the
    // current warp's shmem region, in the layout expected by
    // `BMatrix.Load<ColumnMajor, uint4>`.
    //
    // - For half:  1 uint4 = 8 halfs = 1 tile, so one uint4 store per lane.
    //              The WTV's `Uint4AlignedInputSize` already guarantees
    //              at least 1-uint4 (= 1 tile) of storage past the logical
    //              `InputSize`, so a full-tile read is always in-bounds
    //              and no bounds check is needed.
    // - For float: 1 uint4 = 4 floats = half a tile, so two uint4 stores
    //              per lane. The WTV's storage is 1-uint4 aligned for
    //              float (= half a tile), so if `InputSize` is not a
    //              multiple of MetalTile, the last k-iteration's read
    //              would exceed the caller's logical input.
    //
    // The float path uses a single per-tile branch
    // (`srcStart + MetalTile <= InputSize`) to pick between:
    //   - Fast path: both uint4 stores unchecked. Taken for every tile
    //     when `InputSize` is tile-aligned (the compile-time constant
    //     `InputSize % MetalTile == 0` short-circuits the condition, so
    //     the slow path is dead-code-eliminated), and for all but the
    //     last partial tile otherwise.
    //   - Slow path: per-element bounds check, zero-pad OOB slots so
    //     the MMA's last partial tile sees zeros rather than
    //     uninitialized storage. Runs at most once per `forward` call.
    // =========================================================================

    [ForceInline]
    internal static void packInputTileToShmem<int SrcSize>(
        __constref WaveTangledVector<T, ShMemSize, SrcSize, SubgroupSize, SubgroupCount> input,
        int srcStart,
        uint shMemDstUint4)
    {
        if (sizeof(T) == sizeof(half))
        {
            let u0 = bit_cast<uint2>(half4(
                __realCast<half>(input.data[srcStart + 0]),
                __realCast<half>(input.data[srcStart + 1]),
                __realCast<half>(input.data[srcStart + 2]),
                __realCast<half>(input.data[srcStart + 3])));
            let u1 = bit_cast<uint2>(half4(
                __realCast<half>(input.data[srcStart + 4]),
                __realCast<half>(input.data[srcStart + 5]),
                __realCast<half>(input.data[srcStart + 6]),
                __realCast<half>(input.data[srcStart + 7])));
            ShMemPool.data[shMemDstUint4] = uint4(u0.x, u0.y, u1.x, u1.y);
        }
        else if (SrcSize % MetalTile == 0 || srcStart + MetalTile <= SrcSize)
        {
            // Fast path. Taken when either:
            //   (a) `SrcSize` is tile-aligned, so no contract iteration can
            //       exceed it (compile-time-foldable), or
            //   (b) this tile is fully in-bounds (runtime check, true for
            //       every iteration except the last partial one).
            ShMemPool.data[shMemDstUint4 + 0] = uint4(
                bit_cast<uint>(input.data[srcStart + 0]),
                bit_cast<uint>(input.data[srcStart + 1]),
                bit_cast<uint>(input.data[srcStart + 2]),
                bit_cast<uint>(input.data[srcStart + 3]));
            ShMemPool.data[shMemDstUint4 + 1] = uint4(
                bit_cast<uint>(input.data[srcStart + 4]),
                bit_cast<uint>(input.data[srcStart + 5]),
                bit_cast<uint>(input.data[srcStart + 6]),
                bit_cast<uint>(input.data[srcStart + 7]));
        }
        else
        {
            // Slow path: float's last partial tile on a non-tile-aligned
            // `SrcSize`. Per-element bounds check, zero OOB slots so the
            // MMA accumulator sees zeros rather than uninitialized storage.
            let e0 = (srcStart + 0 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 0]) : 0u;
            let e1 = (srcStart + 1 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 1]) : 0u;
            let e2 = (srcStart + 2 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 2]) : 0u;
            let e3 = (srcStart + 3 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 3]) : 0u;
            let e4 = (srcStart + 4 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 4]) : 0u;
            let e5 = (srcStart + 5 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 5]) : 0u;
            let e6 = (srcStart + 6 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 6]) : 0u;
            let e7 = (srcStart + 7 < SrcSize) ? bit_cast<uint>(input.data[srcStart + 7]) : 0u;
            ShMemPool.data[shMemDstUint4 + 0] = uint4(e0, e1, e2, e3);
            ShMemPool.data[shMemDstUint4 + 1] = uint4(e4, e5, e6, e7);
        }
    }

    // =========================================================================
    // mmaAccumulateMgGroup
    //
    // Compute one M_BLOCK-wide tile group of the output:
    //   acc[mb][n] = sum over contract tiles of A_tile × B_tile
    // where the A tiles come from the weight matrix W (possibly transposed),
    // and the B tiles come from `input` packed into shmem tile-by-tile.
    //
    // Template parameters:
    //   `ContractSize`  — length of `input` in elements; equals the contract
    //                     dimension of the MMA (K for forward, M for
    //                     mmaTranspose).
    //   `DoTransposeA`  — false: A is the W[m, k] row-major tile (forward).
    //                     true:  A is the W^T[k, m] = W[m, k] viewed as
    //                            column-major (mmaTranspose / backward).
    //
    // The weight matrix W is stored in memory as OutputSize × InputSize
    // row-major, so the underlying stride is always `K = InputSize`,
    // regardless of whether we are loading W or W^T.
    //
    // This helper is the shared core of `forward` and `mmaTranspose`.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void mmaAccumulateMgGroup<Address, int ContractSize, bool DoTransposeA>(
        __constref WaveTangledVector<T, ShMemSize, ContractSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress,
        int outerGroup,
        inout CMatrix acc[M_BLOCK][TILES_N_],
        uint perWarpBase,
        uint lane)
        where Address : IPointerLikeAddress<T>
    {
        static const int TILES_C = (ContractSize + MetalTile - 1) / MetalTile;

        for (int mb = 0; mb < M_BLOCK; mb++)
            for (int n = 0; n < TILES_N_; n++)
                acc[mb][n].fill(T(0));

        // Contract loop is NOT unrolled. Tried [ForceUnroll]: helped half
        // slightly (~1%) but caused 16-27% regression at float for
        // ContractSize=128 — fully inlining 16 iterations exceeds the GPU's
        // instruction window / register budget. Code-size grows ~3x.
        for (int c = 0; c < TILES_C; c++)
        {
            // Each lane packs its MetalTile-element slice of `input.data`
            // into `TileWidthUint4_` uint4 stores in shmem. See
            // `packInputTileToShmem`'s doc block for the bounds-check
            // rationale for float with non-tile-aligned `ContractSize`.
            packInputTileToShmem<ContractSize>(
                input,
                c * MetalTile,
                perWarpBase + lane * uint(TileWidthUint4_));
            GroupMemoryBarrierWithWaveSync();

            BMatrix matB[TILES_N_];
            [ForceUnroll]
            for (int n = 0; n < TILES_N_; n++)
            {
                uint tileOffsetUint4 = perWarpBase + uint(n * MetalTile * TileWidthUint4_);
                matB[n] = BMatrix.Load<linalg.CoopMatMatrixLayout.ColumnMajor, uint4>(
                    ShMemPool.data, tileOffsetUint4, uint(TileWidthUint4_));
            }
            // Post-load barrier mirrors fromVectorViaShMem's structure
            // (write -> barrier -> load -> barrier). Empirically, placing
            // the barrier here (instead of post-MMA) lets the GPU schedule
            // MMA more efficiently and is ~5% faster at half on M4.
            GroupMemoryBarrierWithWaveSync();

            [ForceUnroll]
            for (int mb = 0; mb < M_BLOCK; mb++)
            {
                int outerTile = outerGroup * M_BLOCK + mb;

                AMatrix matA;
                if (DoTransposeA)
                {
                    // W^T[outerTile*8+r, c*8+col] = W[c*8+col, outerTile*8+r].
                    // Base address = c*8*K + outerTile*8 (row c*8 of W);
                    // ColumnMajor load with stride=K produces the transposed
                    // 8x8 block as MatrixA.
                    matA = loadCoopMat<T, Address, MetalTile, MetalTile,
                        linalg.CoopMatMatrixUse.MatrixA,
                        linalg.CoopMatMatrixLayout.ColumnMajor>(
                        weightAddress,
                        uint(c * MetalTile * K + outerTile * MetalTile),
                        uint(K));
                }
                else
                {
                    // W[outerTile*8+r, c*8+col].
                    // Base address = outerTile*8*K + c*8; RowMajor stride=K.
                    matA = loadCoopMat<T, Address, MetalTile, MetalTile,
                        linalg.CoopMatMatrixUse.MatrixA,
                        linalg.CoopMatMatrixLayout.RowMajor>(
                        weightAddress,
                        uint(outerTile * MetalTile * K + c * MetalTile),
                        uint(K));
                }

                [ForceUnroll]
                for (int n = 0; n < TILES_N_; n++)
                    acc[mb][n] = linalg.coopMatMulAdd<T, false>(matA, matB[n], acc[mb][n]);
            }
        }
    }

    // =========================================================================
    // writebackMgGroup
    //
    // Flush an M_BLOCK-wide tile group of accumulator back out to a
    // WaveTangledVector via shmem. For each of the M_BLOCK A-rows:
    //   1. simdgroup_store every `TILES_N_` accumulator into a shared
    //      8 rows × (TILES_N_ * MetalTile = N) strip of shmem (row-major).
    //   2. Each lane reads its column from the strip into output.data[...].
    // The shmem region is shared with the B preload used by
    // `mmaAccumulateMgGroup` — safe because by writeback the B-data is dead.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void writebackMgGroup<int OutputN>(
        inout WaveTangledVector<T, ShMemSize, OutputN, SubgroupSize, SubgroupCount> output,
        __constref CMatrix acc[M_BLOCK][TILES_N_],
        int outerGroup,
        Ptr<T, Access::ReadWrite, AddressSpace::GroupShared> shmemPtr,
        uint lane)
    {
        [ForceUnroll]
        for (int mb = 0; mb < M_BLOCK; mb++)
        {
            int outerTile = outerGroup * M_BLOCK + mb;

            [ForceUnroll]
            for (int n = 0; n < TILES_N_; n++)
                acc[mb][n].Store<linalg.CoopMatMatrixLayout.RowMajor>(shmemPtr + n * MetalTile, uint(N));

            GroupMemoryBarrierWithWaveSync();

            [ForceUnroll]
            for (int i = 0; i < MetalTile; i++)
                output.data[outerTile * MetalTile + i] = shmemPtr[i * N + int(lane)];

            GroupMemoryBarrierWithWaveSync();
        }
    }

    // =========================================================================
    // Forward: result(M) = W(M,K) × input(K)
    //
    // Direct CoopMat implementation with a per-warp, uint4-packed shmem
    // layout. Outer loop tiles the M dimension into `M_GROUPS_` groups of
    // `M_BLOCK` rows each; the inner contract loop (over K) is handled by
    // `mmaAccumulateMgGroup`, and writeback by `writebackMgGroup`.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>
    forward<Address, bool Bias = false>(
        __constref WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
    {
        typealias OutputVec = WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>;

        OutputVec output;

        static const int TILES_M_ = (M + MetalTile - 1) / MetalTile;
        static const int M_GROUPS_ = TILES_M_ / M_BLOCK;

        uint lane = WaveGetLaneIndex();
        uint warpId = WaveGetWaveIndex();
        uint perWarpBase = warpId * uint(ShmemPerWarpUint4_);

        Ptr<T, Access::ReadWrite, AddressSpace::GroupShared> shmemPtr =
            bit_cast<Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>>(
                __getAddress(ShMemPool.data[perWarpBase]));

        for (int mg = 0; mg < M_GROUPS_; mg++)
        {
            CMatrix acc[M_BLOCK][TILES_N_];
            mmaAccumulateMgGroup<Address, InputSize, false>(
                input, weightAddress, mg, acc, perWarpBase, lane);
            writebackMgGroup<OutputSize>(output, acc, mg, shmemPtr, lane);
        }

        if (Bias)
        {
            for (int i = 0; i < M; i++)
                output.data[i] = output.data[i] + biasAddress.value[uint(i)];
        }

        return output;
    }

    // =========================================================================
    // mmaTranspose: result(K) = W^T(K, M) × input(M)
    //
    // Used in the backward path to compute dInput = W^T × dOut, where the
    // original forward was output = W × input. The input here is M-sized
    // (forward's output space) and the result is K-sized (forward's input
    // space).
    //
    // The weight matrix W is stored in memory as M×K row-major; loading
    // the transposed W^T tiles is done by `mmaAccumulateMgGroup` with
    // `DoTransposeA=true` (ColumnMajor load with stride=K over swapped
    // row/col offsets). The outer loop tiles the K dimension into
    // `K_GROUPS_` groups of `M_BLOCK` rows each; contract is over M.
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount>
    mmaTranspose<Address>(
        __constref WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress)
        where Address : IPointerLikeAddress<T>
    {
        typealias ResultVec = WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount>;

        ResultVec output;

        static const int TILES_K_ = (K + MetalTile - 1) / MetalTile;
        static const int K_GROUPS_ = TILES_K_ / M_BLOCK;

        uint lane = WaveGetLaneIndex();
        uint warpId = WaveGetWaveIndex();
        uint perWarpBase = warpId * uint(ShmemPerWarpUint4_);

        Ptr<T, Access::ReadWrite, AddressSpace::GroupShared> shmemPtr =
            bit_cast<Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>>(
                __getAddress(ShMemPool.data[perWarpBase]));

        for (int kg = 0; kg < K_GROUPS_; kg++)
        {
            CMatrix acc[M_BLOCK][TILES_N_];
            mmaAccumulateMgGroup<Address, OutputSize, true>(
                input, weightAddress, kg, acc, perWarpBase, lane);
            writebackMgGroup<InputSize>(output, acc, kg, shmemPtr, lane);
        }

        return output;
    }

    // =========================================================================
    // Outer product accumulate: weightGrad(M,K) += sum_lanes(dOut(M) * input(K)^T)
    //
    // This is the standard-module version of the benchmark's "Slang CoopMat
    // direct" path. Each warp stages two 8-row dOut tiles and two 8-column input
    // tiles into threadgroup memory, uses CoopMat MMA to sum across the 32 lanes,
    // then each warp atomic-adds its own accumulator tile to global memory.
    // =========================================================================

    [ForceInline]
    [require(metal, cooperative_matrix, subgroup_basic)]
    internal static void outerProductAccumulate<U, Address>(
        __constref WaveTangledVector<U, ShMemSize, OutputSize, SubgroupSize, SubgroupCount> inputVectorA,
        __constref WaveTangledVector<U, ShMemSize, InputSize, SubgroupSize, SubgroupCount> inputVectorB,
        Address weightAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        typealias OPAAMatrix = linalg.CoopMat<U, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixA>;
        typealias OPABMatrix = linalg.CoopMat<U, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixB>;
        typealias OPACMatrix = linalg.CoopMat<U, MemoryScope.Subgroup, MetalTile, MetalTile, linalg.CoopMatMatrixUse.MatrixAccumulator>;

        static const int OPA_N_BLOCK = 2;
        static const int TILES_M_ = OutputSize / MetalTile;
        static const int TILES_K_ = InputSize / MetalTile;
        static const int TILES_LANE_ = SubgroupSize / MetalTile;

        static const int StageAElemsPerWarp = M_BLOCK * MetalTile * SubgroupSize;
        static const int StageBElemsPerWarp = OPA_N_BLOCK * MetalTile * SubgroupSize;
        static const int AccumElemsPerWarp = MetalTile * MetalTile;
        static const int PerWarpElems = StageAElemsPerWarp + StageBElemsPerWarp + AccumElemsPerWarp;

        uint lane = WaveGetLaneIndex();
        uint warpId = WaveGetWaveIndex();
        uint perWarpBase = warpId * uint(PerWarpElems);
        uint stageABase = perWarpBase;
        uint stageBBase = stageABase + uint(StageAElemsPerWarp);
        uint accumBase = stageBBase + uint(StageBElemsPerWarp);

        Ptr<U, Access::ReadWrite, AddressSpace::GroupShared> shmemPtr =
            bit_cast<Ptr<U, Access::ReadWrite, AddressSpace::GroupShared>>(
                __getAddress(ShMemPool.data[0]));

        for (int mg = 0; mg < TILES_M_ / M_BLOCK; mg++)
        {
            for (int kg = 0; kg < TILES_K_ / OPA_N_BLOCK; kg++)
            {
                [ForceUnroll]
                for (int mb = 0; mb < M_BLOCK; mb++)
                {
                    int m = mg * M_BLOCK + mb;
                    [ForceUnroll]
                    for (int row = 0; row < MetalTile; row++)
                    {
                        shmemPtr[stageABase + uint(mb * MetalTile * SubgroupSize + row * SubgroupSize) + lane] =
                            inputVectorA.data[m * MetalTile + row];
                    }
                }

                [ForceUnroll]
                for (int kb = 0; kb < OPA_N_BLOCK; kb++)
                {
                    int kTile = kg * OPA_N_BLOCK + kb;
                    [ForceUnroll]
                    for (int col = 0; col < MetalTile; col++)
                    {
                        shmemPtr[stageBBase + uint(kb * MetalTile * SubgroupSize + col * SubgroupSize) + lane] =
                            inputVectorB.data[kTile * MetalTile + col];
                    }
                }
                GroupMemoryBarrierWithGroupSync();

                OPACMatrix acc[M_BLOCK][OPA_N_BLOCK];
                [ForceUnroll]
                for (int mb = 0; mb < M_BLOCK; mb++)
                    [ForceUnroll]
                    for (int kb = 0; kb < OPA_N_BLOCK; kb++)
                        acc[mb][kb].fill(U(0));

                [ForceUnroll]
                for (int laneTile = 0; laneTile < TILES_LANE_; laneTile++)
                {
                    OPAAMatrix matA[M_BLOCK];
                    [ForceUnroll]
                    for (int mb = 0; mb < M_BLOCK; mb++)
                    {
                        matA[mb] = OPAAMatrix.Load<linalg.CoopMatMatrixLayout.RowMajor>(
                            shmemPtr + int(stageABase) + mb * MetalTile * SubgroupSize + laneTile * MetalTile,
                            uint(SubgroupSize));
                    }

                    OPABMatrix matB[OPA_N_BLOCK];
                    [ForceUnroll]
                    for (int kb = 0; kb < OPA_N_BLOCK; kb++)
                    {
                        matB[kb] = OPABMatrix.Load<linalg.CoopMatMatrixLayout.ColumnMajor>(
                            shmemPtr + int(stageBBase) + kb * MetalTile * SubgroupSize + laneTile * MetalTile,
                            uint(SubgroupSize));
                    }

                    [ForceUnroll]
                    for (int mb = 0; mb < M_BLOCK; mb++)
                        [ForceUnroll]
                        for (int kb = 0; kb < OPA_N_BLOCK; kb++)
                            acc[mb][kb] = linalg.coopMatMulAdd<U, false>(matA[mb], matB[kb], acc[mb][kb]);
                }

                [ForceUnroll]
                for (int mb = 0; mb < M_BLOCK; mb++)
                {
                    int m = mg * M_BLOCK + mb;
                    [ForceUnroll]
                    for (int kb = 0; kb < OPA_N_BLOCK; kb++)
                    {
                        int kTile = kg * OPA_N_BLOCK + kb;
                        acc[mb][kb].Store<linalg.CoopMatMatrixLayout.RowMajor>(
                            shmemPtr + int(accumBase), uint(MetalTile));
                        GroupMemoryBarrierWithGroupSync();

                        if (sizeof(U) == sizeof(half))
                        {
                            for (uint idx = lane; idx < uint(MetalTile * MetalTile / 2); idx += uint(SubgroupSize))
                            {
                                uint i = idx / uint(MetalTile / 2);
                                uint j = idx % uint(MetalTile / 2);
                                uint r = uint(m * MetalTile) + i;
                                uint c = uint(kTile * MetalTile) + j * 2;
                                weightAddress.atomicAdd(
                                    r * uint(InputSize) + c,
                                    vector<U, 2>(
                                        shmemPtr[accumBase + i * uint(MetalTile) + j * 2],
                                        shmemPtr[accumBase + i * uint(MetalTile) + j * 2 + 1]));
                            }
                        }
                        else
                        {
                            for (uint idx = lane; idx < uint(MetalTile * MetalTile); idx += uint(SubgroupSize))
                            {
                                uint r = uint(m * MetalTile) + idx / uint(MetalTile);
                                uint c = uint(kTile * MetalTile) + idx % uint(MetalTile);
                                weightAddress.atomicAdd(
                                    r * uint(InputSize) + c,
                                    shmemPtr[accumBase + idx]);
                            }
                        }
                        GroupMemoryBarrierWithGroupSync();
                    }
                }
            }
        }
    }

    // =========================================================================
    // Bias reduce: dBias += sum(dOut) across lanes.
    //
    // Prototype Metal implementation using the simple wave-reduce strategy:
    // each warp reduces one output element across its lanes, writes the per-warp
    // partial into shmem, then warp 0 lane 0 reduces those partials and atomicAdds
    // to the global bias buffer. This mirrors the benchmark's "wave shmem" path.
    // =========================================================================

    [ForceInline]
    [require(metal, subgroup_basic)]
    internal static void sumReduceRows<U, Address, InArrayType>(
        uint sharedMemoryOffset,
        __constref WaveTangledVector<U, ShMemSize, OutputSize, SubgroupSize, SubgroupCount> dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        uint lane = WaveGetLaneIndex();

        Ptr<U, Access::ReadWrite, AddressSpace::GroupShared> shmemPtr =
            bit_cast<Ptr<U, Access::ReadWrite, AddressSpace::GroupShared>>(
                __getAddress(ShMemPool.data[sharedMemoryOffset]));

        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
        {
            U reduced = WaveActiveSum(dOut.data[i]);
            if (lane == 0)
                shmemPtr[subgroupId] = reduced;
            GroupMemoryBarrierWithGroupSync();

            if (subgroupId == 0 && lane == 0)
            {
                U total = U(0);
                [ForceUnroll]
                for (int w = 0; w < SubgroupCount; w++)
                    total += shmemPtr[w];
                biasAddress.atomicAdd(uint(i), total);
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }

}
