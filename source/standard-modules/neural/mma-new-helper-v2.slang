implementing neural;

#include "common-def.slang"

// MMAHelperNewV2: Tin2-style tiled MMA for backward pass.
//
// Key architectural difference from MMAHelperNew (V1):
//   Vectors are stored as MatA with row=WarpSize (thread identity).
//   This means MMA results naturally have row=thread, so to_array
//   extracts directly without a transpose+ChangeMajor step.
//
// Weight tiles are MatB, loaded in native fragment layout.
// The transpose MMA uses: MatA(dOut, 32×M) × MatB(W^T, M×K) → MatA(dIn, 32×K).
// The outer product uses: MatA(dOut^T, M×32) × MatB(input, 32×K) → MatA(dW, M×K).
//
// MMA shape: m16n16k16 (internally 2× m16n8k16)

[require(cuda)]
VISIBILITY_LEVEL struct MMAHelperNewV2<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int M = OutputSize;
    static const int K = InputSize;
    static const int N = SubgroupSize;  // WarpSize = 32

    static const int ROW = 16;
    static const int COL = 16;

    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);

    VISIBILITY_LEVEL typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, ROW, COL, linalg.CoopMatMatrixUse.MatrixA>;
    VISIBILITY_LEVEL typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, ROW, COL, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T, MemoryScope.Subgroup, ROW, COL, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    // Vector tile counts: MatA with row=N (WarpSize=32), col=element_count
    // MMARows = N/16, MMACols = element_count/16
    static const int NRowTiles = (N + ROW - 1) / ROW;  // 32/16 = 2

    VISIBILITY_LEVEL static const int DOutTileCount = NRowTiles * ((M + COL - 1) / COL);
    VISIBILITY_LEVEL static const int InputTileCount = NRowTiles * ((K + COL - 1) / COL);

    // Weight: MatB with shape (M×K) for forward, stored in native tiled layout
    static const int WeightTileRows = (M + ROW - 1) / ROW;
    static const int WeightTileCols = (K + COL - 1) / COL;
    static const int WeightTileCount = WeightTileRows * WeightTileCols;

    // Transpose MMA result: MatA(32×K)
    VISIBILITY_LEVEL static const int ResultTileCount = NRowTiles * ((K + COL - 1) / COL);

    // Outer product result: MatA(M×K), then transposed to MatB(K×M)
    static const int OPResultRows = (M + ROW - 1) / ROW;
    static const int OPResultCols = (K + COL - 1) / COL;
    static const int OPResultTileCount = OPResultRows * OPResultCols;

    // Shmem tile stride for cross-warp reduction (uint4 units per tile)
    static const int OPA_ElementsPerLane = (ROW * COL) / N;
    static const int OPA_RegsPerLane = (OPA_ElementsPerLane * sizeof(T) + 3) / 4;
    static const int OPA_TileStride = (OPA_RegsPerLane * N * 4 + sizeof(uint4) - 1) / sizeof(uint4);

    // =========================================================================
    // from_array / to_array: convert between per-thread half[] and MatA tiles
    // =========================================================================

    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void fromArrayPacked<int PackedSize, int OutN>(
        uint packedData[PackedSize],
        inout MatA outArray[OutN])
    {
        uint laneid = WaveGetLaneIndex();

        static const int TotalColsPacked = PackedSize;
        static const int TileColsPacked = COL / 2;
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

    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void toArrayPacked<int PackedSize, int InN>(
        MatA inArray[InN],
        inout uint packedOut[PackedSize])
    {
        uint laneid = WaveGetLaneIndex();

            static const int TotalColsPacked = PackedSize;
            static const int TileColsPacked = COL / 2;
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
                    regsi[i] = inArray[base_frag_idx].fragmentRead(base_reg_idx);

                    [ForceUnroll]
                    for (uint j = 1; j < RegColsPacked; j++)
                    {
                        uint r_row = j % RegRows;
                        uint mma_row = j / RegRows;

                        int reg_idx = int(r_col * RegRows + r_row);
                        int frag_idx = int(mma_col * MMARows + mma_row);

                        if (laneid % RegColsPacked == (i + j) % RegColsPacked)
                            regsi[i] = inArray[frag_idx].fragmentRead(reg_idx);
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
    // Tile-level transpose: swap reg[1] <-> reg[2] within each tile,
    // plus swap off-diagonal tiles in the grid.
    // Equivalent to Tin2's .transpose() (logical, no movmatrix).
    // =========================================================================

    [ForceInline]
    static void transposeTileGrid<int Rows, int Cols>(inout MatA tiles[Rows * Cols])
    {
        // Swap reg[1] <-> reg[2] within each tile (sub-tile transpose)
        [ForceUnroll]
        for (int idx = 0; idx < Rows * Cols; idx++)
        {
            uint r1 = tiles[idx].fragmentRead(1);
            uint r2 = tiles[idx].fragmentRead(2);
            tiles[idx].fragmentWrite(1, r2);
            tiles[idx].fragmentWrite(2, r1);
        }

        // Swap off-diagonal tiles: tile(r,c) <-> tile(c,r) for r != c
        // Tiles are stored column-major: idx = col * Rows + row
        static const int MinDim = Rows < Cols ? Rows : Cols;
        [ForceUnroll]
        for (int r = 0; r < MinDim; r++)
        {
            [ForceUnroll]
            for (int c = r + 1; c < (Rows < Cols ? Cols : Rows); c++)
            {
                if (r < Rows && c < Cols && c < Rows && r < Cols)
                {
                    int idx_rc = c * Rows + r;
                    int idx_cr = r * Rows + c;
                    MatA tmp = tiles[idx_rc];
                    tiles[idx_rc] = tiles[idx_cr];
                    tiles[idx_cr] = tmp;
                }
            }
        }
    }

    // =========================================================================
    // changeMajorAxis: MatA -> MatB with movmatrix per tile + tile grid remap.
    // Equivalent to Tin2's change_major_axis().
    // =========================================================================

    [ForceInline]
    static void changeMajorAxisA2B<int Rows, int Cols>(
        MatA input[Rows * Cols],
        inout MatB output[Rows * Cols])
    {
        // Tin2 iterates: for i in [0,MMACols), for j in [0,MMARows)
        //   src_idx = x.mma_rc_to_idx(j, i) = j * MMACols + i  (ROW_MAJOR=true)
        //   dst_idx = r.mma_rc_to_idx(j, i) = i * MMARows + j  (ROW_MAJOR=false)
        //   dst[dst_idx] = movmatrix(src[src_idx])
        [ForceUnroll]
        for (int i = 0; i < Cols; i++)
        {
            [ForceUnroll]
            for (int j = 0; j < Rows; j++)
            {
                int src_idx = j * Cols + i;
                int dst_idx = i * Rows + j;
                output[dst_idx].copyFrom(input[src_idx]);
                output[dst_idx].ChangeMajor();
            }
        }
    }

    // changeMajorAxis: MatB -> MatA (reverse direction)
    [ForceInline]
    static void changeMajorAxisB2A<int Rows, int Cols>(
        MatB input[Rows * Cols],
        inout MatA output[Rows * Cols])
    {
        [ForceUnroll]
        for (int i = 0; i < Cols; i++)
        {
            [ForceUnroll]
            for (int j = 0; j < Rows; j++)
            {
                int src_idx = i * Rows + j;
                int dst_idx = j * Cols + i;
                output[dst_idx].copyFrom(input[src_idx]);
                output[dst_idx].ChangeMajor();
            }
        }
    }

    // =========================================================================
    // Weight loading: load MatB tiles from native tiled layout in global memory
    // =========================================================================

    [ForceInline]
    [require(cuda)]
    static void loadWeightTiles<U, Address>(
        Address weightAddress,
        inout MatB tiles[WeightTileCount])
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        static const int TileSizeInElements = ROW * COL;
        [ForceUnroll]
        for (int r = 0; r < WeightTileRows; r++)
        {
            [ForceUnroll]
            for (int c = 0; c < WeightTileCols; c++)
            {
                int tileIdx = c * WeightTileRows + r;
                int offset = (r * WeightTileCols + c) * TileSizeInElements;
                let tileAddr = weightAddress.getOffset(offset);
                tiles[tileIdx] = MatB.LoadRaw(bit_cast<RWStructuredBuffer<half>>(tileAddr.getBuffer()), tileAddr.getBaseIndex());
            }
        }
    }

    // =========================================================================
    // Transpose MMA: dIn = W^T × dOut
    //
    // dOut is MatA(32×M), W is MatB(M×K) loaded native.
    // W^T needs: .transpose() + changeMajorAxis → MatB(K×M... wait no)
    //
    // Tin2 flow:
    //   w: MatB(M×K) → w.transpose(): MatA(K×M) → change_major_axis: MatB(K×M)
    //   But we need MMA: A(32×M) × B(M×K_transposed)
    //   Actually: dOut is MatA(32×M). W^T means we want B with K-dim=M.
    //
    // MMA: C[32×K] = A[32×M] × B[M×K]
    // We need B = W^T reshaped as MatB(M × K).
    // But W is stored as MatB(M×K) in native layout (row=M, col=K).
    // W^T would be MatB(K×M) or MatA(M×K).
    //
    // Following Tin2 exactly:
    //   w: MatB(M×K)
    //   w.transpose(): MatA(K×M) ← type flip + reg swap
    //   change_major_axis(...): MatB(K×M) ← movmatrix
    //   mul(dOut: MatA(32×M), w_t: MatB(M×K_transposed))
    //
    // Wait, Tin2's backward has Z0=InputSize=32, Z1=OutputSize=16:
    //   w: HMatrixB<Z0,Z1> = MatB(32,16) = MatB(K,M)
    //   w.transpose(): MatA(M,K) = MatA(16,32)
    //   change_major_axis: MatB(M,K) = MatB(16,32)
    //   w_t: MatB(16,32) = MatB(M,K)
    //
    //   mul(op_grad: MatA(32,M), w_t: MatB(M,K)) → MatA(32,K)
    //   MMA K-dim here is M (inner/shared dim)
    //
    // So the MMA tiles:
    //   A tiles: dOut has NRowTiles × (M/16) tiles
    //   B tiles: w_t has (M/16) × (K/16) tiles (K-dim = M in MMA notation)
    //   C tiles: result has NRowTiles × (K/16) tiles
    //
    // The MMA loop iterates over the shared dimension (M):
    //   for k_tile in [0, M/16):
    //     for i in [0, NRowTiles):
    //       for j in [0, K/16):
    //         C[i][j] += A[i][k_tile] × B[k_tile][j]
    // =========================================================================

    static const int TransposeMMA_ATileCols = (M + COL - 1) / COL;  // shared dim tiles
    static const int TransposeMMA_BTileRows = (M + ROW - 1) / ROW;  // shared dim tiles (should == ATileCols)
    static const int TransposeMMA_BTileCols = (K + COL - 1) / COL;  // output dim tiles
    static const int TransposeMMA_CTileRows = NRowTiles;
    static const int TransposeMMA_CTileCols = (K + COL - 1) / COL;

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void mmaTranspose<U, Address>(
        MatA dOutTiles[DOutTileCount],
        Address weightAddress,
        inout MatA resultTiles[ResultTileCount])
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        // Load weight as MatB(K×M) in native tiled layout
        // Note: weight is stored as M×K in row-major tile order,
        // but loaded here as MatB which interprets as K×M in Tin2's convention.
        MatB wTiles[WeightTileCount];
        loadWeightTiles<U, Address>(weightAddress, wTiles);

        // Transpose: MatB(K×M) → MatA(M×K) via transpose (reg swap + tile grid swap)
        MatA wTilesA[WeightTileCount];
        [ForceUnroll]
        for (int i = 0; i < WeightTileCount; i++)
            wTilesA[i].copyFrom(wTiles[i]);

        // Reg 1<->2 swap within each tile
        [ForceUnroll]
        for (int i = 0; i < WeightTileCount; i++)
        {
            uint r1 = wTilesA[i].fragmentRead(1);
            uint r2 = wTilesA[i].fragmentRead(2);
            wTilesA[i].fragmentWrite(1, r2);
            wTilesA[i].fragmentWrite(2, r1);
        }

        // Tile grid swap for non-square grids:
        // wTiles was col-major with (WeightTileRows × WeightTileCols)
        // After transpose, becomes (WeightTileCols × WeightTileRows) col-major
        // We need to remap tile indices accordingly
        // For the changeMajorAxis below, we need the tiles in the transposed grid order

        // changeMajorAxis: MatA(M×K) → MatB(M×K)
        MatB wtTilesB[WeightTileCount];
        // After transpose, the MatA has shape (M/16 rows × K/16 cols)
        // = (WeightTileRows × WeightTileCols) but with swapped tile positions
        changeMajorAxisA2B<TransposeMMA_BTileRows, TransposeMMA_BTileCols>(wTilesA, wtTilesB);

        // Initialize result accumulators
        MatC matC[ResultTileCount];
        [ForceUnroll]
        for (int i = 0; i < ResultTileCount; i++)
            matC[i].clear();

        // MMA: C[NRowTiles × K/16] = A[NRowTiles × M/16] × B[M/16 × K/16]
        [ForceUnroll]
        for (int k = 0; k < TransposeMMA_ATileCols; k++)
        {
            [ForceUnroll]
            for (int j = 0; j < TransposeMMA_CTileCols; j++)
            {
                [ForceUnroll]
                for (int i = 0; i < TransposeMMA_CTileRows; i++)
                {
                    int a_idx = k * NRowTiles + i;     // A is col-major: col=k, row=i
                    int b_idx = j * TransposeMMA_BTileRows + k;  // B is col-major: col=j, row=k
                    int c_idx = j * TransposeMMA_CTileRows + i;  // C is col-major: col=j, row=i

                    matC[c_idx] = linalg.coopMatMulAdd<T, false, half, half, T,
                                    MemoryScope.Subgroup, ROW, COL, COL
                                    >(dOutTiles[a_idx], wtTilesB[b_idx], matC[c_idx]);
                }
            }
        }

        // Copy MatC → MatA (same register layout for f16)
        [ForceUnroll]
        for (int i = 0; i < ResultTileCount; i++)
            resultTiles[i].copyFrom(matC[i]);
    }

    // =========================================================================
    // Outer product: dW = dOut ⊗ input (summed across threads via MMA)
    //
    // Tin2 flow:
    //   at = change_major_axis(input):    MatA(32×K) → MatB(32×K)
    //   bt = change_major_axis(dOut.transpose()): MatA(32×M) → transpose → MatA(M×32) → CMA → MatA(M×32)
    //   Wait, re-reading Tin2: bt = change_major_axis(b.transpose())
    //     b = HVector<M> = MatA(32×M, ROW_MAJOR=true)
    //     b.transpose() = MatrixBase(M×32, ROW_MAJOR=false) = MatB-like
    //     change_major_axis(MatB(M×32)) → MatA(M×32)
    //
    //   w = mul(bt: MatA(M×32), at: MatB(32×K)) → MatA(M×K)
    //   wt = w.transpose() → MatB(K×M) (free reg swap)
    //
    // MMA: C[M×K] = A[M×32] × B[32×K], shared dim = 32 = WarpSize
    // This sums over threads (the K-dim of MMA = thread identity).
    // =========================================================================

    static const int OP_ATileRows = (M + ROW - 1) / ROW;
    static const int OP_ATileCols = NRowTiles;              // 32/16 = 2
    static const int OP_BTileRows = NRowTiles;              // 32/16 = 2
    static const int OP_BTileCols = (K + COL - 1) / COL;
    static const int OP_CTileRows = (M + ROW - 1) / ROW;
    static const int OP_CTileCols = (K + COL - 1) / COL;
    VISIBILITY_LEVEL static const int OP_TotalTiles = OP_CTileRows * OP_CTileCols;

    // Store a MatC tile to shmem
    [ForceInline]
    static void storeTileToShmem<ShMemSize : ISharedMemorySize>(
        MatC tile, uint offset)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        uint laneId = WaveGetLaneIndex();

        if (OPA_RegsPerLane <= 4)
        {
            uint4 v = uint4(tile.fragmentRead(0), tile.fragmentRead(1),
                            tile.fragmentRead(2), tile.fragmentRead(3));
            ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId), v);
        }
        else
        {
            uint4 v0 = uint4(tile.fragmentRead(0), tile.fragmentRead(1),
                             tile.fragmentRead(2), tile.fragmentRead(3));
            uint4 v1 = uint4(tile.fragmentRead(4), tile.fragmentRead(5),
                             tile.fragmentRead(6), tile.fragmentRead(7));
            ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2), v0);
            ShMemPool.smemStore<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2 + 1), v1);
        }
    }

    // Load a MatC tile from shmem
    [ForceInline]
    static MatC loadTileFromShmem<ShMemSize : ISharedMemorySize>(uint offset)
    {
        typealias ShMemPool = SharedMemoryPool<ShMemSize>;
        uint laneId = WaveGetLaneIndex();
        MatC tile;

        if (OPA_RegsPerLane <= 4)
        {
            uint4 v = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId));
            tile.fragmentWrite(0, v.x);
            tile.fragmentWrite(1, v.y);
            tile.fragmentWrite(2, v.z);
            tile.fragmentWrite(3, v.w);
        }
        else
        {
            uint4 v0 = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2));
            uint4 v1 = ShMemPool.smemLoad<uint4, ShmemAddressMode.Shared>(offset, int(laneId * 2 + 1));
            tile.fragmentWrite(0, v0.x);
            tile.fragmentWrite(1, v0.y);
            tile.fragmentWrite(2, v0.z);
            tile.fragmentWrite(3, v0.w);
            tile.fragmentWrite(4, v1.x);
            tile.fragmentWrite(5, v1.y);
            tile.fragmentWrite(6, v1.z);
            tile.fragmentWrite(7, v1.w);
        }
        return tile;
    }

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void outerProduct<U, Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        inout MatA dOutTiles[DOutTileCount],
        inout MatA inputTiles[InputTileCount],
        Address weightAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        const uint subgroupIndex = getWaveId();

        // changeMajorAxis(input): ChangeMajor in-place (param is by-value, safe to modify)
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputTiles[i].ChangeMajor();

        // dOut transpose + changeMajorAxis in-place
        [ForceUnroll]
        for (int i = 0; i < DOutTileCount; i++)
        {
            uint r1 = dOutTiles[i].fragmentRead(1);
            uint r2 = dOutTiles[i].fragmentRead(2);
            dOutTiles[i].fragmentWrite(1, r2);
            dOutTiles[i].fragmentWrite(2, r1);
            dOutTiles[i].ChangeMajor();
        }

        // Convert input to MatB up front (like Tin2)
        MatB inputB[InputTileCount];
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputB[i].copyFrom(inputTiles[i]);

        // MMA: C[M×K] = A[M×32] × B[32×K]
        MatC matC[OP_TotalTiles];
        [ForceUnroll]
        for (int idx = 0; idx < OP_TotalTiles; idx++)
            matC[idx].clear();

        [ForceUnroll]
        for (int k = 0; k < OP_ATileCols; k++)
        {
            [ForceUnroll]
            for (int j = 0; j < OP_CTileCols; j++)
            {
                [ForceUnroll]
                for (int i = 0; i < OP_CTileRows; i++)
                {
                    int a_idx = k * OP_ATileRows + i;
                    int b_idx = j * OP_BTileRows + k;
                    int c_idx = j * OP_CTileRows + i;

                    matC[c_idx] = linalg.coopMatMulAdd<T, false, half, half, T,
                                    MemoryScope.Subgroup, ROW, COL, COL
                                    >(dOutTiles[a_idx], inputB[b_idx], matC[c_idx]);
                }
            }
        }

        // Cross-warp reduction
        if (SubgroupCount > 1)
        {
            for (int s = 1; s < SubgroupCount; s <<= 1)
            {
                uint subgroupOffset = (subgroupIndex / (s << 1)) * (OP_TotalTiles * OPA_TileStride);
                uint base = sharedMemoryOffset + subgroupOffset;

                if (subgroupIndex % (s << 1) == uint(s))
                {
                    [ForceUnroll]
                    for (int idx = 0; idx < OP_TotalTiles; idx++)
                        storeTileToShmem<ShMemSize>(matC[idx], base + idx * OPA_TileStride);
                }

                GroupMemoryBarrierWithGroupSync();

                if (subgroupIndex % (s << 1) == 0)
                {
                    if (subgroupIndex != uint(SubgroupCount - 1))
                    {
                        [ForceUnroll]
                        for (int idx = 0; idx < OP_TotalTiles; idx++)
                        {
                            let other = loadTileFromShmem<ShMemSize>(base + idx * OPA_TileStride);
                            matC[idx] = matC[idx] + other;
                        }
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

            [ForceUnroll]
            for (int idx = 0; idx < OP_TotalTiles; idx++)
                storeTileToShmem<ShMemSize>(matC[idx], sharedMemoryOffset + idx * OPA_TileStride);

            GroupMemoryBarrierWithWaveSync();

            if (sizeof(T) == 2)
            {
                static const int H2PerTile = OPA_TileStride * (sizeof(uint4) / sizeof(U)) / 2;
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

    [ForceInline]
    static uint checksumMatC(inout MatC matC[OP_TotalTiles])
    {
        uint checksum = 0;
        [ForceUnroll]
        for (int idx = 0; idx < OP_TotalTiles; idx++)
        {
            if (OPA_RegsPerLane <= 4)
            {
                checksum += matC[idx].fragmentRead(0);
                checksum += matC[idx].fragmentRead(1);
                checksum += matC[idx].fragmentRead(2);
                checksum += matC[idx].fragmentRead(3);
            }
            else
            {
                checksum += matC[idx].fragmentRead(0);
                checksum += matC[idx].fragmentRead(1);
                checksum += matC[idx].fragmentRead(2);
                checksum += matC[idx].fragmentRead(3);
                checksum += matC[idx].fragmentRead(4);
                checksum += matC[idx].fragmentRead(5);
                checksum += matC[idx].fragmentRead(6);
                checksum += matC[idx].fragmentRead(7);
            }
        }
        return checksum;
    }

    // Diagnostic wrapper: keep only the outer-product transform + MMA work alive.
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static uint outerProductComputeChecksum(
        inout MatA dOutTiles[DOutTileCount],
        inout MatA inputTiles[InputTileCount])
    {
        // changeMajorAxis(input): ChangeMajor in-place (param is by-value, safe to modify)
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputTiles[i].ChangeMajor();

        // dOut transpose + changeMajorAxis in-place
        [ForceUnroll]
        for (int i = 0; i < DOutTileCount; i++)
        {
            uint r1 = dOutTiles[i].fragmentRead(1);
            uint r2 = dOutTiles[i].fragmentRead(2);
            dOutTiles[i].fragmentWrite(1, r2);
            dOutTiles[i].fragmentWrite(2, r1);
            dOutTiles[i].ChangeMajor();
        }

        MatB inputB[InputTileCount];
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputB[i].copyFrom(inputTiles[i]);

        MatC matC[OP_TotalTiles];
        [ForceUnroll]
        for (int idx = 0; idx < OP_TotalTiles; idx++)
            matC[idx].clear();

        [ForceUnroll]
        for (int k = 0; k < OP_ATileCols; k++)
        {
            [ForceUnroll]
            for (int j = 0; j < OP_CTileCols; j++)
            {
                [ForceUnroll]
                for (int i = 0; i < OP_CTileRows; i++)
                {
                    int a_idx = k * OP_ATileRows + i;
                    int b_idx = j * OP_BTileRows + k;
                    int c_idx = j * OP_CTileRows + i;

                    matC[c_idx] = linalg.coopMatMulAdd<T, false, half, half, T,
                                    MemoryScope.Subgroup, ROW, COL, COL
                                    >(dOutTiles[a_idx], inputB[b_idx], matC[c_idx]);
                }
            }
        }

        return checksumMatC(matC);
    }

    // Diagnostic wrapper: keep outer-product transform + MMA + cross-warp reduce alive.
    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static uint outerProductComputeReduceChecksum<ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        inout MatA dOutTiles[DOutTileCount],
        inout MatA inputTiles[InputTileCount])
    {
        const uint subgroupIndex = getWaveId();

        // changeMajorAxis(input): ChangeMajor in-place (param is by-value, safe to modify)
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputTiles[i].ChangeMajor();

        // dOut transpose + changeMajorAxis in-place
        [ForceUnroll]
        for (int i = 0; i < DOutTileCount; i++)
        {
            uint r1 = dOutTiles[i].fragmentRead(1);
            uint r2 = dOutTiles[i].fragmentRead(2);
            dOutTiles[i].fragmentWrite(1, r2);
            dOutTiles[i].fragmentWrite(2, r1);
            dOutTiles[i].ChangeMajor();
        }

        MatB inputB[InputTileCount];
        [ForceUnroll]
        for (int i = 0; i < InputTileCount; i++)
            inputB[i].copyFrom(inputTiles[i]);

        MatC matC[OP_TotalTiles];
        [ForceUnroll]
        for (int idx = 0; idx < OP_TotalTiles; idx++)
            matC[idx].clear();

        [ForceUnroll]
        for (int k = 0; k < OP_ATileCols; k++)
        {
            [ForceUnroll]
            for (int j = 0; j < OP_CTileCols; j++)
            {
                [ForceUnroll]
                for (int i = 0; i < OP_CTileRows; i++)
                {
                    int a_idx = k * OP_ATileRows + i;
                    int b_idx = j * OP_BTileRows + k;
                    int c_idx = j * OP_CTileRows + i;

                    matC[c_idx] = linalg.coopMatMulAdd<T, false, half, half, T,
                                    MemoryScope.Subgroup, ROW, COL, COL
                                    >(dOutTiles[a_idx], inputB[b_idx], matC[c_idx]);
                }
            }
        }

        if (SubgroupCount > 1)
        {
            for (int s = 1; s < SubgroupCount; s <<= 1)
            {
                uint subgroupOffset = (subgroupIndex / (s << 1)) * (OP_TotalTiles * OPA_TileStride);
                uint base = sharedMemoryOffset + subgroupOffset;

                if (subgroupIndex % (s << 1) == uint(s))
                {
                    [ForceUnroll]
                    for (int idx = 0; idx < OP_TotalTiles; idx++)
                        storeTileToShmem<ShMemSize>(matC[idx], base + idx * OPA_TileStride);
                }

                GroupMemoryBarrierWithGroupSync();

                if (subgroupIndex % (s << 1) == 0)
                {
                    if (subgroupIndex != uint(SubgroupCount - 1))
                    {
                        [ForceUnroll]
                        for (int idx = 0; idx < OP_TotalTiles; idx++)
                        {
                            let other = loadTileFromShmem<ShMemSize>(base + idx * OPA_TileStride);
                            matC[idx] = matC[idx] + other;
                        }
                    }
                }

                GroupMemoryBarrierWithGroupSync();
            }
        }

        if (subgroupIndex == 0)
            return checksumMatC(matC);
        return 0u;
    }

    // =========================================================================
    // Bias reduce: dBias = sum(dOut) across threads
    // Same approach as V1: reduce MatA tiles via register adds + shuffles + shmem.
    // =========================================================================

    static const int NBiasTiles = (M + COL - 1) / COL;
    static const int NRowGroups = NRowTiles;
    VISIBILITY_LEVEL static const int TotalBiasTilesMatA = NBiasTiles * NRowGroups;

    [ForceInline]
    [require(cuda, subgroup_shuffle)]
    VISIBILITY_LEVEL static void biasReduce<U, Address, ShMemSize : ISharedMemorySize, int SubgroupCount>(
        uint sharedMemoryOffset,
        MatA dOutTiles[DOutTileCount],
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

        MatA localTiles[DOutTileCount];
        [ForceUnroll]
        for (int i = 0; i < DOutTileCount; i++)
            localTiles[i] = dOutTiles[i];

        [ForceUnroll]
        for (int tc = 0; tc < NBiasTiles; tc++)
        {
            [ForceUnroll]
            for (int g = 1; g < NRowGroups; g++)
            {
                [ForceUnroll]
                for (int r = 0; r < 4; r++)
                {
                    uint v0 = localTiles[tc * NRowGroups].fragmentRead(r);
                    uint v1 = localTiles[tc * NRowGroups + g].fragmentRead(r);
                    localTiles[tc * NRowGroups].fragmentWrite(r, bit_cast<uint>(bit_cast<half2>(v0) + bit_cast<half2>(v1)));
                }
            }

            {
                uint r0 = localTiles[tc * NRowGroups].fragmentRead(0);
                uint r1 = localTiles[tc * NRowGroups].fragmentRead(1);
                uint r2 = localTiles[tc * NRowGroups].fragmentRead(2);
                uint r3 = localTiles[tc * NRowGroups].fragmentRead(3);

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

        // Cross-warp reduction
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
