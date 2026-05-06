// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;


#define Max(A, B) ((A) > (B) ? (A) : (B))

public interface ISharedMemorySize
{
    static const uint Bytes;
}

// Address mode for shared memory store/load operations.
//
// On CUDA, shared memory can be accessed via two paths:
//
// 1. Shared (32-bit): Uses st.shared.v4.u32 / ld.shared.v4.u32 inline asm
//    with a 32-bit shared address. This generates STS.128 / LDS.128 in SASS.
//    Pros: Fewest instructions (no address conversion overhead).
//    Cons: Inline asm is opaque to ptxas — it cannot decompose, reorder, or
//          reschedule around the instruction. For vectorized v4 stores, all 4
//          data registers must be live simultaneously, which increases register
//          pressure and can cause more spilling in register-starved kernels.
//
// 2. Generic (64-bit): Converts the 32-bit shared address to a 64-bit generic
//    pointer via cvta.shared.u64, then dereferences through normal C++ pointer
//    assignment (*ptr = value). This generates ST.E.128 / LD.E.128 in SASS.
//    Pros: ptxas has full scheduling freedom — it can split, reorder, or
//          interleave stores with other work, reducing register pressure.
//    Cons: 3 extra address-computation instructions per access (IADD3 + IADD3
//          + IMAD.X to build the 64-bit address), making total instruction
//          count higher.
//
// For register-heavy kernels (e.g. 255 registers), Generic mode typically wins
// because the scheduling freedom reduces spilling more than the extra address
// instructions cost. For smaller kernels with register headroom, Shared mode
// is more efficient due to fewer total instructions.
internal enum ShmemAddressMode
{
    Shared,
    Generic,
}

public struct SharedMemoryPool<ShMemSize: ISharedMemorySize>
{
    internal static groupshared uint4 data[ShMemSize.Bytes / sizeof(uint4)];

    // =========================================================================
    // Pack/unpack helpers: store/load scalar U into/from uint4 array by index.
    //
    // Layout: element index `i` of type U is stored at:
    //   uint4 slot:  i / ElementsPerUint4
    //   uint  comp:  (i % ElementsPerUint4) / ElementsPerUint
    //   sub-element: i % ElementsPerUint  (shift = subIdx * sizeof(U) * 8)
    // =========================================================================

    [ForceInline]
    [require(cuda)]
    static uint getSharedBaseAddr<let V : int>(groupshared uint4[V] shmem)
    {
        __intrinsic_asm "(unsigned)__cvta_generic_to_shared(($0).m_data)";
    }

    [ForceInline]
    [require(cuda)]
    static U smemLoadShared32<U>(uint addr)
        where U : IArithmetic
    {
        switch (sizeof(U))
        {
        case 2:
            __intrinsic_asm R"(
                ([&]{
                    unsigned short _r;
                    asm("ld.shared.u16 %0, [%1];" : "=h"(_r) : "r"($0));
                    return *reinterpret_cast<$TR*>(&_r);
                }())
            )";
        case 4:
            __intrinsic_asm R"(
                ([&]{
                    unsigned _r;
                    asm("ld.shared.u32 %0, [%1];" : "=r"(_r) : "r"($0));
                    return *reinterpret_cast<$TR*>(&_r);
                }())
            )";
        case 8:
            __intrinsic_asm R"(
                ([&]{
                    unsigned _r[2];
                    asm("ld.shared.v2.u32 {%0, %1}, [%2];" : "=r"(_r[0]), "=r"(_r[1]) : "r"($0));
                    return *reinterpret_cast<$TR*>(_r);
                }())
            )";
        case 16:
            __intrinsic_asm R"(
                ([&]{
                    unsigned _r[4];
                    asm("ld.shared.v4.u32 {%0, %1, %2, %3}, [%4];" : "=r"(_r[0]), "=r"(_r[1]), "=r"(_r[2]), "=r"(_r[3]) : "r"($0));
                    return *reinterpret_cast<$TR*>(_r);
                }())
            )";
        default:
            static_assert(false, "smemLoadShared32: unsupported sizeof(U)");
            return U(0);
        }
    }

    [ForceInline]
    [require(cuda)]
    static void smemStoreShared32<U>(uint addr, U value)
        where U : IArithmetic
    {
        switch (sizeof(U))
        {
        case 2:
            __intrinsic_asm R"(
                {
                    unsigned short _v = *reinterpret_cast<const unsigned short*>(&$1);
                    asm("st.shared.u16 [%0], %1;" :: "r"($0), "h"(_v));
                }
            )";
        case 4:
            __intrinsic_asm R"(
                {
                    unsigned _v = *reinterpret_cast<const unsigned*>(&$1);
                    asm("st.shared.u32 [%0], %1;" :: "r"($0), "r"(_v));
                }
            )";
        case 8:
            __intrinsic_asm R"(
                {
                    const unsigned* _t = reinterpret_cast<const unsigned*>(&$1);
                    asm("st.shared.v2.u32 [%0], {%1, %2};" :: "r"($0), "r"(_t[0]), "r"(_t[1]));
                }
            )";
        case 16:
            __intrinsic_asm R"(
                {
                    const unsigned* _t = reinterpret_cast<const unsigned*>(&$1);
                    asm("st.shared.v4.u32 [%0], {%1, %2, %3, %4};" :: "r"($0), "r"(_t[0]), "r"(_t[1]), "r"(_t[2]), "r"(_t[3]));
                }
            )";
        default:
            static_assert(false, "smemStoreShared32: unsupported sizeof(U)");
        }
    }

    // Generic address helpers: convert 32-bit shared addr → 64-bit generic ptr → dereference.
    // Gives ptxas freedom to reschedule (generates st.v4.u32 / ld.v4.u32 via generic addressing).
    [ForceInline]
    [require(cuda)]
    static void smemStoreViaGenericPtr<U>(uint addr, U value)
        where U : IArithmetic
    {
        __intrinsic_asm R"(
            ([&]() {
                size_t _a64 = (size_t)$0;
                unsigned* _p;
                asm("cvta.shared.u64 %0, %1;" : "=l"(_p) : "l"(_a64));
                *reinterpret_cast<$T1*>(_p) = $1;
            }())
        )";
    }

    [ForceInline]
    [require(cuda)]
    static U smemLoadViaGenericPtr<U>(uint addr)
        where U : IArithmetic
    {
        __intrinsic_asm R"(
            ([&]() {
                size_t _a64 = (size_t)$0;
                const unsigned* _p;
                asm("cvta.shared.u64 %0, %1;" : "=l"(_p) : "l"(_a64));
                return *reinterpret_cast<const $TR*>(_p);
            }())
        )";
    }

    [ForceInline]
    static void smemStore<U, ShmemAddressMode Mode>(uint memOffset, int index, U value)
        where U : IArithmetic
    {
        __target_switch
        {
        case cuda:
            {
                uint addr = getSharedBaseAddr(data) + memOffset * sizeof(uint4) + index * sizeof(U);
                if (Mode == ShmemAddressMode.Generic)
                    smemStoreViaGenericPtr<U>(addr, value);
                else
                    smemStoreShared32<U>(addr, value);
            }
        default:
            smemStoreGeneric<U>(memOffset, index, value);
        }
    }

    [ForceInline]
    static void smemStore<U>(uint memOffset, int index, U value)
        where U : IArithmetic
    {
        smemStore<U, ShmemAddressMode.Shared>(memOffset, index, value);
    }

    [ForceInline]
    static U smemLoad<U, ShmemAddressMode Mode>(uint memOffset, int index)
        where U : IArithmetic
    {
        __target_switch
        {
        case cuda:
            {
                uint addr = getSharedBaseAddr(data) + memOffset * sizeof(uint4) + index * sizeof(U);
                if (Mode == ShmemAddressMode.Generic)
                    return smemLoadViaGenericPtr<U>(addr);
                else
                    return smemLoadShared32<U>(addr);
            }
        default:
            return smemLoadGeneric<U>(memOffset, index);
        }
    }

    [ForceInline]
    static U smemLoad<U>(uint memOffset, int index)
        where U : IArithmetic
    {
        return smemLoad<U, ShmemAddressMode.Shared>(memOffset, index);
    }

    // Generic fallback for non-CUDA targets (SPIR-V etc.)
    [ForceInline]
    static void smemStoreGeneric<U>(uint memOffset, int index, U value)
        where U : IArithmetic
    {
        switch (sizeof(U))
        {
        case 2:
            {
                static const int elementsPerUint  = 4 / sizeof(U);
                static const int elementsPerUint4 = 16 / sizeof(U);
                static const int bitsPerElement   = sizeof(U) * 8;
                const int slot = index / elementsPerUint4;
                const int comp = (index % elementsPerUint4) / elementsPerUint;
                const int sub  = index % elementsPerUint;
                const uint shift = sub * bitsPerElement;
                uint packed = data[memOffset + slot][comp];
                packed = (packed & ~(uint(0xFFFF) << shift)) | (uint(bit_cast<uint16_t>(value)) << shift);
                data[memOffset + slot][comp] = packed;
            }
            break;
        case 4:
            {
                static const int elementsPerUint  = 4 / sizeof(U);
                static const int elementsPerUint4 = 16 / sizeof(U);
                const int slot = index / elementsPerUint4;
                const int comp = (index % elementsPerUint4) / elementsPerUint;
                data[memOffset + slot][comp] = bit_cast<uint>(value);
            }
            break;
        case 8:
            {
                const int slot8 = index / 2;
                const int comp8 = (index % 2) * 2;
                vector<uint, 2> v = bit_cast<vector<uint, 2>>(value);
                data[memOffset + slot8][comp8] = v[0];
                data[memOffset + slot8][comp8 + 1] = v[1];
            }
            break;
        case 16:
            data[memOffset + index] = bit_cast<uint4>(value);
            break;
        default:
            static_assert(false, "smemStoreGeneric: unsupported sizeof(U)");
        }
    }

    [ForceInline]
    static U smemLoadGeneric<U>(uint memOffset, int index)
        where U : IArithmetic
    {
        switch (sizeof(U))
        {
        case 2:
            {
                static const int elementsPerUint  = 4 / sizeof(U);
                static const int elementsPerUint4 = 16 / sizeof(U);
                static const int bitsPerElement   = sizeof(U) * 8;
                const int slot = index / elementsPerUint4;
                const int comp = (index % elementsPerUint4) / elementsPerUint;
                const int sub  = index % elementsPerUint;
                const uint shift = sub * bitsPerElement;
                return bit_cast<U>(uint16_t((data[memOffset + slot][comp] >> shift) & 0xFFFF));
            }
        case 4:
            {
                static const int elementsPerUint  = 4 / sizeof(U);
                static const int elementsPerUint4 = 16 / sizeof(U);
                const int slot = index / elementsPerUint4;
                const int comp = (index % elementsPerUint4) / elementsPerUint;
                return bit_cast<U>(data[memOffset + slot][comp]);
            }
        case 8:
            {
                const int slot8 = index / 2;
                const int comp8 = (index % 2) * 2;
                uint lo = data[memOffset + slot8][comp8];
                uint hi = data[memOffset + slot8][comp8 + 1];
                return bit_cast<U>(vector<uint, 2>(lo, hi));
            }
        case 16:
            return bit_cast<U>(data[memOffset + index]);
        default:
            static_assert(false, "smemLoadGeneric: unsupported sizeof(U)");
            return U(0);
        }
    }
}

internal struct SharedMemoryUsage<T, TargetEnum Target, ExecutionMode ExeMode, int InputSize, int OutputSize, int SubgroupSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const bool IsTraining = ExeMode == ExecutionMode.Training;
    typealias TileInfoNormal = TileInfo<T, OutputSize, SubgroupSize, InputSize, Target, false>;
    typealias TileInfoTransposed = TileInfo<T, OutputSize, SubgroupSize, InputSize, Target, true>;
    typealias CMShape = CoopMatShape<T, Target>;

    // Shared memory A is used to load Tile A. The Size Tile A is determined by the height of matrix A and width of CoopMatA.
    // The possible shapes of matrix A can be:
    // 1. M x K in A * B      -> inference (TransposeA = false)
    // 2. K x M in A^T * B    -> training (TransposeA = true)
    // 3. M x N in outer product of dOut and input. -> training (TransposeA = false)
    // In the inference mode, tile A size is always [M x CoopMatA_Width].
    // In the training mode, tile A size is either [M x CoopMatA_Width] or [K x CoopMatA_Width], so we need to choose the max value
    static const int SharedMemSizeInVectorMatA = !IsTraining ?
        (TileInfoNormal.HeightInElementsTileA * CMShape.COLUMN_A) / CMShape.ElementCountPerVector :
        (Max(TileInfoNormal.HeightInElementsTileA, TileInfoTransposed.HeightInElementsTileA) * CMShape.COLUMN_A) / CMShape.ElementCountPerVector;

    // Shared memory B is used to load Tile B. The Size Tile B is determined by the height of CoopMatB and width of Tile B.
    // The possible shapes of matrix B in inference mode can be:
    // 1. K x N in A * B      -> inference
    // 2. M x N in A^T * B    -> training
    // 3. N x K in outer product of dOut and input. -> training
    // In the inference mode, tile B size is always [CoopMatB_Height x N].
    // In the training mode, tile B size is either [CoopMatB_Height x N] or [CoopMatB_Height x K], so we need to choose the max value.

    // InputSize is K.
    static const int TileBWidthForOuterProduct = ((InputSize + CMShape.COLUMN_B - 1) / CMShape.COLUMN_B) * CMShape.COLUMN_B;
    static const int SharedMemSizeInVectorMatB = !IsTraining ?
        ((TileInfoNormal.WidthInElementsTileB * CMShape.ROW_B) / CMShape.ElementCountPerVector) :
        ((Max(TileInfoNormal.WidthInElementsTileB, TileBWidthForOuterProduct) * CMShape.ROW_B) / CMShape.ElementCountPerVector);

    // Shared memory C is used to store the result of CoopMatC. The size is determened by height of CoopMatC and width of Tile C.
    // The possible shapes matrix C can only be:
    // 1. M x N in A * B
    // 2. K x N in A^T * B
    // 3. M x K in outer product of dOut and input.
    // Therefore the Tile C size is same as the Tile B size. However, the data type of Tile B can only be half, while tile C can be
    // both float and half, so we need to take that into account.
    static const int SharedMemSizeInVectorMatC = SharedMemSizeInVectorMatB * sizeof(T) / sizeof(half);
}

internal struct SharedMemorySizeForLayer<
    T,
    TargetEnum Target,
    ExecutionMode ExeMode,
    int SubgroupSize,
    int SubgroupCount,
    int InputSize,
    int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemInfo = SharedMemoryUsage<T, Target, ExeMode, InputSize, OutputSize, SubgroupSize>;
    typealias CMShape = CoopMatShape<T, Target>;

    // The MMA forward/backward uses: shared A tiles + per-warp B/C tiles.
    // With MMAHelperNew (CUDA + OptimalLayout), forward and backward MMA
    // use zero shmem (LoadRaw + FromLocal). This term only applies to
    // the legacy shmem-based MMAHelper/TiledMMAHelper paths (non-CUDA).
    //
    // Metal forward/backward transpose: on-demand per-k fromVectorViaShMem for
    // both half and float. Shmem per warp = SubgroupSize * TileWidthUint4 (in
    // uint4 units). Half: 8 halfs / 8 per uint4 = 1 uint4 per lane. Float:
    // 8 floats / 4 per uint4 = 2 uint4s per lane (stored as bit_cast, not
    // converted to half).
    static const int MetalTile_ = 8;
    static const int MetalElemsPerUint4_ = sizeof(uint4) / sizeof(T);
    static const int MetalTileWidthUint4_ = (MetalTile_ + MetalElemsPerUint4_ - 1) / MetalElemsPerUint4_;
    static const uint MetalBytesForMMA =
        uint(SubgroupCount) * uint(SubgroupSize * MetalTileWidthUint4_) * sizeof(uint4);

    // Metal outer-product backward stages two 8-row dOut tiles, two 8-column
    // input tiles, and one 8x8 accumulator tile per warp in T-typed shmem.
    static const int MetalOPMBlock_ = 2;
    static const int MetalOPNBlock_ = 2;
    static const int MetalOPStageAElemsPerWarp_ = MetalOPMBlock_ * MetalTile_ * SubgroupSize;
    static const int MetalOPStageBElemsPerWarp_ = MetalOPNBlock_ * MetalTile_ * SubgroupSize;
    static const int MetalOPAccumElemsPerWarp_ = MetalTile_ * MetalTile_;
    static const uint MetalBytesForOuterProduct =
        !ShMemInfo.IsTraining ? 0 :
        uint(SubgroupCount) *
            uint(MetalOPStageAElemsPerWarp_ + MetalOPStageBElemsPerWarp_ + MetalOPAccumElemsPerWarp_) *
            sizeof(T);

    static const uint BytesForMMA = (Target == TargetEnum.Metal)
        ? Max(MetalBytesForMMA, MetalBytesForOuterProduct)
        : (ShMemInfo.SharedMemSizeInVectorMatA + (ShMemInfo.SharedMemSizeInVectorMatC) * SubgroupCount) * sizeof(uint4);

    // Cross-warp butterfly reduction for outer product.
    // The first butterfly pass has G/2 warps storing simultaneously, each
    // needing one full MatrixC<T,M,K> in native layout. The final store-to-
    // global phase (warp 0 only) also needs one matrix. Max(G/2, 1) covers both.
    static const int NTileRows = (OutputSize + CMShape.ROW_A - 1) / CMShape.ROW_A;
    static const int NTileCols = (InputSize + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
    static const int ElementsPerLane = CMShape.SizeCInElements / SubgroupSize;
    static const int RegsPerLane = (ElementsPerLane * sizeof(T) + 3) / 4;
    static const int TileStride = (RegsPerLane * SubgroupSize * 4 + sizeof(uint4) - 1) / sizeof(uint4);
    static const uint BytesForCrossWarpReduction = !ShMemInfo.IsTraining ? 0 :
        Max(SubgroupCount / 2, 1) * NTileRows * NTileCols * TileStride * sizeof(uint4);

    // Outer product store phase: one row of MatC tiles stored to shmem at a time.
    // NTileCols MatC tiles × (ROW_C * COLUMN_C * sizeof(T)) bytes each.
    static const int NCoopMatCColumns = (InputSize + CMShape.COLUMN_B - 1) / CMShape.COLUMN_B;
    static const uint BytesForOuterProductStore = !ShMemInfo.IsTraining ? 0 :
        NCoopMatCColumns * CMShape.ROW_C * CMShape.COLUMN_C * sizeof(T);

    // sumReduceRows uses three contiguous shared memory regions:
    //   1. Per-warp input vectors: SubgroupCount * SubgroupSize * HeightInVecB uint4s
    //   2. Per-warp matC temp:     SubgroupCount * TileStrideVec * ROW_C uint4s
    //   3. Cross-warp reduction:   enough to hold SubgroupCount * MPadded elements of T
    static const int HeightInVecB_SR = CMShape.ROW_B / CMShape.ElementCountPerVector;
    static const int TileStrideVec_SR = CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;
    static const int MPadded_SR = (OutputSize + 1) & ~1;
    static const uint BytesForSumReduceRows = !ShMemInfo.IsTraining ? 0 :
        (SubgroupCount * SubgroupSize * HeightInVecB_SR + SubgroupCount * TileStrideVec_SR * CMShape.ROW_C) * sizeof(uint4)
        + ((SubgroupCount * MPadded_SR * sizeof(T) + sizeof(uint4) - 1) / sizeof(uint4)) * sizeof(uint4);

    public static const uint Bytes = Max(Max(Max(BytesForMMA, BytesForOuterProductStore),
                                          BytesForCrossWarpReduction),
                                                BytesForSumReduceRows);
}

internal interface ISharedMemorySizePackNode
{
    static const uint Bytes;
}

internal struct SharedMemorySizePackEnd : ISharedMemorySizePackNode
{
    static const uint Bytes = 0;
}

internal struct SharedMemorySizePackNode<
    T,
    TargetEnum Target,
    ExecutionMode ExeMode,
    int SubgroupSize,
    int SubgroupCount,
    int CurrentLayerSize,
    int NextLayerSize,
    let each RestLayerSize : int>
    : ISharedMemorySizePackNode
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const uint Current =
        SharedMemorySizeForLayer<
            T,
            Target,
            ExeMode,
            SubgroupSize,
            SubgroupCount,
            CurrentLayerSize,
            NextLayerSize>.Bytes;
    typealias Tail = __packBranch(
        RestLayerSize,
        SharedMemorySizePackEnd,
        SharedMemorySizePackNode<
            T,
            Target,
            ExeMode,
            SubgroupSize,
            SubgroupCount,
            NextLayerSize,
            __first(RestLayerSize),
            __trimFirst(RestLayerSize)>);

    static const uint Bytes = Max(Current, Tail.Bytes);
}

public struct SharedMemorySize<
    T,
    TargetEnum Target,
    ExecutionMode ExeMode,
    int SubgroupSize,
    int SubgroupCount,
    int FirstLayerSize,
    int SecondLayerSize,
    let each RestLayerSize : int>
    : ISharedMemorySize
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public static const uint Bytes =
        SharedMemorySizePackNode<
            T,
            Target,
            ExeMode,
            SubgroupSize,
            SubgroupCount,
            FirstLayerSize,
            SecondLayerSize,
            RestLayerSize>.Bytes;
}
