// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#include "common-def.slang"

#define Max(A, B) ((A) > (B) ? (A) : (B))

internal typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

internal interface ISharedMemoryPool
{
    associatedtype MemSize: ISharedMemorySize;
}

public interface ISharedMemorySize
{
    static const uint Bytes;
}

public struct SharedMemoryPool<ShMemSize: ISharedMemorySize> : ISharedMemoryPool
{
    typealias MemSize = ShMemSize;
}

VISIBILITY_LEVEL extension<T : ISharedMemoryPool> T
{
    VISIBILITY_LEVEL static groupshared uint4 data[T.MemSize.Bytes / sizeof(uint4)];
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

    // Notice that in the actual implementation, we always reuse shared memory for Tile B and Tile C because they are always used at
    // different stages of the computation, and they have the same size.
    public static const uint Bytes =
        (ShMemInfo.SharedMemSizeInVectorMatA + (ShMemInfo.SharedMemSizeInVectorMatC) * SubgroupCount) * sizeof(uint4);
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
