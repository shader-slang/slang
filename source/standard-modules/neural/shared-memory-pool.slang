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

public struct SharedMemorySize0<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount, int InputSize, int OutputSize>
                    : ISharedMemorySize
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemInfo = SharedMemoryUsage<T, Target, ExeMode, InputSize, OutputSize, SubgroupSize>;
    typealias CMShape = CoopMatShape<T, Target>;

    // The MMA forward/backward uses: shared A tiles + per-warp B/C tiles.
    // With MMAHelperNew (CUDA + OptimalLayout), forward and backward MMA
    // use zero shmem (LoadRaw + FromLocal). This term only applies to
    // the legacy shmem-based MMAHelper/TiledMMAHelper paths (non-CUDA).
    static const uint BytesForMMA =
        (ShMemInfo.SharedMemSizeInVectorMatA + (ShMemInfo.SharedMemSizeInVectorMatC) * SubgroupCount) * sizeof(uint4);

    // Cross-warp all-at-once reduction for sumReduceTilesAllAtOnce().
    // Stores ALL tiles at once instead of row-by-row to minimize barriers.
    // Memory = NTileRows * NTileCols * TileStride * sizeof(uint4)
    // Used by both the legacy outer product (shmem-staged) and the new
    // MMAHelperNew outer product (FromLocal-based, shmem only for reduction).
    static const int NTileRows = (OutputSize + CMShape.ROW_A - 1) / CMShape.ROW_A;
    static const int NTileCols = (InputSize + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
    static const int ElementsPerLane = CMShape.SizeCInElements / SubgroupSize;
    static const int RegsPerLane = (ElementsPerLane * sizeof(T) + 3) / 4;
    static const int TileStride = (RegsPerLane * SubgroupSize * 4 + sizeof(uint4) - 1) / sizeof(uint4);
    static const uint BytesForCrossWarpReduction = !ShMemInfo.IsTraining ? 0 :
        NTileRows * NTileCols * TileStride * sizeof(uint4);

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

    public static const uint Bytes = Max(Max(Max(BytesForMMA, BytesForOuterProductStore), BytesForCrossWarpReduction), BytesForSumReduceRows);
}

// The following code is a macro-based implementation of the shared memory size calculation.
// It is used to calculate the shared memory size for a given number of hidden layers.
// The challenge here is that the size of the shared memory has to be compile time constant, however
// slang doesn't really have const_expr function. So the only way to get the compile time constant
// is to use meta programming to generate the code that can be evaluated at compile time.
// Here the algorithm is very simple where we just use divide and conquer to calculate the shared memory size.
// Firstly, we define the base case `SharedMemorySize0` that give an input and output of a layer, we calculate the shared memory size for this layer.
// Then we can define larger number of layers by using the divide and conquer strategy. The reason to use Macro here
// is just to reduce the mount of code that we need to write. But under the hood, the macro will be expanded to:
// `SharedMemorySize1` to `SharedMemorySize15`.

// Take an example:
// ```
// DEFINE_SHMEM_SIZE(3, 1, 1, PARAM_3, ARG_3_L, ARG_3_R)
// ```
// This will be expanded to:
// ```
// public struct SharedMemorySize3<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount, uint S0, uint S1, uint S2, uint S3> SHMEM_WHERE {
//     internal static const uint a = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, S0, S1, S2>.Bytes;
//     internal static const uint b = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, S2, S3, S4>.Bytes;
//     public static const uint Bytes = Max(a, b);
// }
// ```
// Where `LN` and `RN` determine how we divide input sequence of layers into two parts.

// TODO: We shouldn't need such sophisticated meta-programming to achieve this once we have const_expr function
//       or we can provide more advanced variadic generic parameters support such as First(...)/Rest(...), so that
//       we can define the SharedMemorySize as variadic generic struct instead of these pre-defined generics.
//
// We note that this implementation is not the most efficient way to calculate the shared memory size, because
// we can first find out the max layer size, and then do the remaining calculation. But since this computation is
// not done at run time, so we don't need to worry about the performance, and we can reuse other data structure we
// already have, so it's easiest way to implement this.


#define UNPACK(...) __VA_ARGS__

// 2. Define your helper macros
#define SHMEM_WHERE where T : __BuiltinFloatingPointType where T.Differential == T
#define SHMEM_BASE T, Target, ExeMode, SubgroupSize, SubgroupCount

// 3. The Core Macro - note the removed spaces around UNPACK
#define DEFINE_SHMEM_SIZE(N, LN, RN, ARGS, L_VALS, R_VALS)                                                                           \
    public struct SharedMemorySize##N<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount, UNPACK ARGS> \
         : ISharedMemorySize                                                                                                         \
        SHMEM_WHERE                                                                                                                  \
    {                                                                                                                                \
        internal static const uint a = SharedMemorySize##LN<SHMEM_BASE, UNPACK L_VALS>.Bytes;                                        \
        internal static const uint b = SharedMemorySize##RN<SHMEM_BASE, UNPACK R_VALS>.Bytes;                                        \
        public static const uint Bytes = Max(a, b);                                                                                  \
    }

#define PARAM_1 (uint S0, uint S1, uint S2)
#define ARG_1 (S0, S1, S2)
#define ARG_1_L (S0, S1)
#define ARG_1_R (S1, S2)

#define PARAM_2 (UNPACK PARAM_1, uint S3)
#define ARG_2 (UNPACK ARG_1, S3)
#define ARG_2_L (S0, S1, S2)
#define ARG_2_R (S2, S3)

#define PARAM_3 (UNPACK PARAM_2, uint S4)
#define ARG_3 (UNPACK ARG_2, S4)
#define ARG_3_L (S0, S1, S2)
#define ARG_3_R (S2, S3, S4)

DEFINE_SHMEM_SIZE(1, 0, 0, PARAM_1, ARG_1_L, ARG_1_R)
DEFINE_SHMEM_SIZE(2, 1, 0, PARAM_2, ARG_2_L, ARG_2_R)
DEFINE_SHMEM_SIZE(3, 1, 1, PARAM_3, ARG_3_L, ARG_3_R)

// from 4 to 7
#define PARAM_4 (UNPACK PARAM_3, uint S5)
#define ARG_4 (UNPACK ARG_3, S5)
#define ARG_4_R (S4, S5)

#define PARAM_5 (UNPACK PARAM_4, uint S6)
#define ARG_5 (UNPACK ARG_4, S6)
#define ARG_5_R (UNPACK ARG_4_R, S6)

#define PARAM_6 (UNPACK PARAM_5, uint S7)
#define ARG_6 (S0, S1, S2, S3, S4, S5, S6, S7)
#define ARG_6_R (UNPACK ARG_5_R, S7)

#define PARAM_7 (UNPACK PARAM_6, uint S8)
#define ARG_7 (UNPACK ARG_6, S8)
#define ARG_7_R (UNPACK ARG_6_R, S8)

DEFINE_SHMEM_SIZE(4, 3, 0, PARAM_4, ARG_3, ARG_4_R)
DEFINE_SHMEM_SIZE(5, 3, 1, PARAM_5, ARG_3, ARG_5_R)
DEFINE_SHMEM_SIZE(6, 3, 2, PARAM_6, ARG_3, ARG_6_R)
DEFINE_SHMEM_SIZE(7, 3, 3, PARAM_7, ARG_3, ARG_7_R)

// from 8 to 15
#define PARAM_8 (UNPACK PARAM_7, uint S9)
#define ARG_8 (UNPACK ARG_7, S9)
#define ARG_8_R (S8, S9)

#define PARAM_9 (UNPACK PARAM_8, uint S10)
#define ARG_9 (UNPACK ARG_8, S10)
#define ARG_9_R (UNPACK ARG_8_R, S10)

#define PARAM_10 (UNPACK PARAM_9, uint S11)
#define ARG_10 (UNPACK ARG_9, S11)
#define ARG_10_R (UNPACK ARG_9_R, S11)

#define PARAM_11 (UNPACK PARAM_10, uint S12)
#define ARG_11 (UNPACK ARG_10, S12)
#define ARG_11_R (UNPACK ARG_10_R, S12)

#define PARAM_12 (UNPACK PARAM_11, uint S13)
#define ARG_12 (UNPACK ARG_11, S13)
#define ARG_12_R (UNPACK ARG_11_R, S13)

#define PARAM_13 (UNPACK PARAM_12, uint S14)
#define ARG_13 (UNPACK ARG_12, S14)
#define ARG_13_R (UNPACK ARG_12_R, S14)

#define PARAM_14 (UNPACK PARAM_13, uint S15)
#define ARG_14 (UNPACK ARG_13, S15)
#define ARG_14_R (UNPACK ARG_13_R, S15)

#define PARAM_15 (UNPACK PARAM_14, uint S16)
#define ARG_15 (UNPACK ARG_14, S16)
#define ARG_15_R (UNPACK ARG_14_R, S16)

DEFINE_SHMEM_SIZE(8,  7, 0, PARAM_8,  ARG_7, ARG_8_R)
DEFINE_SHMEM_SIZE(9,  7, 1, PARAM_9,  ARG_7, ARG_9_R)
DEFINE_SHMEM_SIZE(10, 7, 2, PARAM_10, ARG_7, ARG_10_R)
DEFINE_SHMEM_SIZE(11, 7, 3, PARAM_11, ARG_7, ARG_11_R)
DEFINE_SHMEM_SIZE(12, 7, 4, PARAM_12, ARG_7, ARG_12_R)
DEFINE_SHMEM_SIZE(13, 7, 5, PARAM_13, ARG_7, ARG_13_R)
DEFINE_SHMEM_SIZE(14, 7, 6, PARAM_14, ARG_7, ARG_14_R)
DEFINE_SHMEM_SIZE(15, 7, 7, PARAM_15, ARG_7, ARG_15_R)

// Slang doesn't support generic overloading, so we cannot provide the one generic with different number of parameters.
public struct SharedMemorySize<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    #define STRIP_PARENS(x) STRIP_PARENS_I x
    #define STRIP_PARENS_I(...) __VA_ARGS__

    public typealias OfLayer1<uint S0, uint S1> = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, S0, S1>;
    public typealias OfLayer2<STRIP_PARENS(PARAM_1)>   = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_1)>;
    public typealias OfLayer3<STRIP_PARENS(PARAM_2)>   = SharedMemorySize2<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_2)>;
    public typealias OfLayer4<STRIP_PARENS(PARAM_3)>   = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_3)>;
    public typealias OfLayer5<STRIP_PARENS(PARAM_4)>   = SharedMemorySize4<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_4)>;
    public typealias OfLayer6<STRIP_PARENS(PARAM_5)>   = SharedMemorySize5<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_5)>;
    public typealias OfLayer7<STRIP_PARENS(PARAM_6)>   = SharedMemorySize6<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_6)>;
    public typealias OfLayer8<STRIP_PARENS(PARAM_7)>   = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_7)>;
    public typealias OfLayer9<STRIP_PARENS(PARAM_8)>   = SharedMemorySize8<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_8)>;
    public typealias OfLayer10<STRIP_PARENS(PARAM_9)>   = SharedMemorySize9<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_9)>;
    public typealias OfLayer11<STRIP_PARENS(PARAM_10)> = SharedMemorySize10<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_10)>;
    public typealias OfLayer12<STRIP_PARENS(PARAM_11)> = SharedMemorySize11<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_11)>;
    public typealias OfLayer13<STRIP_PARENS(PARAM_12)> = SharedMemorySize12<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_12)>;
    public typealias OfLayer14<STRIP_PARENS(PARAM_13)> = SharedMemorySize13<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_13)>;
    public typealias OfLayer15<STRIP_PARENS(PARAM_14)> = SharedMemorySize14<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_14)>;
    public typealias OfLayer16<STRIP_PARENS(PARAM_15)> = SharedMemorySize15<T, Target, ExeMode, SubgroupSize, SubgroupCount, STRIP_PARENS(ARG_15)>;
}

#if 0

// We should implement First/Rest syntax to something like this.
interface IVal {
    static const int Value;
}
SharedMemorySize<int In, each Val:IVal HiddenSize, int OutputSize>
{
    static const uint SharedMemSizeInBytes =
        max(SharedMemorySize<In, First HiddenSize>, SharedMemorySize<each Rest HiddenSize, OutputSize>);
}

// Slang doesn't support generic overloading, therefore we cannot provide the pre-defined generics that adds different number of HiddenSize.

public struct SharedMemorySize<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount, int InputSize, int HiddenSize, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemInfo = SharedMemoryUsage<T, Target, ExeMode, InputSize, OutputSize, SubgroupSize>;

    // Notice that in the actual implementation, we always reuse shared memory for Tile B and Tile C because they are always used at
    // different stages of the computation, and they have the same size.
    static const uint SharedMemSizeInBytes =
        (ShMemInfo.SharedMemSizeInVectorMatA + (ShMemInfo.SharedMemSizeInVectorMatB) * SubgroupCount) * sizeof(uint4);
}
#endif
