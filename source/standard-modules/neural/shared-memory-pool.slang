// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#define Max(A, B) ((A) > (B) ? (A) : (B))

internal struct SharedMemoryUsage<T, TargetEnum Target, ExecutionMode ExeMode, int InputSize, int OutputSize, int SubgroupSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const bool TransposeA = ExeMode == ExecutionMode.Training;
    typealias TileInfo = TileInfo<T, OutputSize, SubgroupSize, InputSize, Target, TransposeA>;
    typealias CMShape = CoopMatShape<T, Target>;

    // Shared memory A is used to load Tile A. The Size Tile A is determined by the height of matrix A and width of CoopMatA.
    // 'TileInfo.HeightInElementsTileA' is the height of the Tile A and it already takes the transpose into account, where it's M
    // for non-transpose case and K for transpose case. 'CMShape.COLUMN_A' is the width of the CoopMatA.
    static const int SharedMemSizeInVectorMatA = (TileInfo.HeightInElementsTileA * CMShape.COLUMN_A) / CMShape.ElementCountPerVector;

    // Shared memory B is used to load Tile B. The Size Tile B is determined by the height of CoopMatB and width of Tile B.
    // The possible shapes of matrix B can be:
    // 1. K x N in A * B
    // 2. M x N in A^T * B
    // 3. N x K in outer product of dOut and input.
    //    In the outer product, matrix B will be collection of input vectors in row major, the row dimension is the batch dimension.
    // Notice that in the inference mode, the width of Tile B is always subgroup size (N). While in the training mode, the width of Tile B is
    // is N and K, so we need to choose the max value.
    static const int TileBWidthForOuterProduct = ((InputSize + CMShape.COLUMN_B - 1) / CMShape.COLUMN_B) * CMShape.COLUMN_B;
    static const int SharedMemSizeInVectorMatB = !TransposeA ?
        ((TileInfo.WidthInElementsTileB * CMShape.ROW_B) / CMShape.ElementCountPerVector) :
        ((Max(TileInfo.WidthInElementsTileB, TileBWidthForOuterProduct) * CMShape.ROW_B) / CMShape.ElementCountPerVector);

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
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemInfo = SharedMemoryUsage<T, Target, ExeMode, InputSize, OutputSize, SubgroupSize>;

    // Notice that in the actual implementation, we always reuse shared memory for Tile B and Tile C because they are always used at
    // different stages of the computation, and they have the same size.
    public static const uint SharedMemSizeInBytes =
        (ShMemInfo.SharedMemSizeInVectorMatA + (ShMemInfo.SharedMemSizeInVectorMatC) * SubgroupCount) * sizeof(uint4);
}





public struct SharedMemorySize1<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize2< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize2, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize3< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize2, HiddenSize3, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize4< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize4, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize5< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
    int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4, int HiddenSize5, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize4, HiddenSize5, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize6<T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize2<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize4, HiddenSize5, HiddenSize6, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize7< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize8< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize9< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize1<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize10< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9, int HiddenSize10, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize2<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize11< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9, int HiddenSize10, int HiddenSize11, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize3<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize12< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9,
                                int HiddenSize10, int HiddenSize11, int HiddenSize12, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize4<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, HiddenSize12, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize13< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9,
                                int HiddenSize10, int HiddenSize11, int HiddenSize12, int HiddenSize13, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize5<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, HiddenSize12, HiddenSize13, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize14< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9,
                                int HiddenSize10, int HiddenSize11, int HiddenSize12, int HiddenSize13, int HiddenSize14, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize6<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, HiddenSize12, HiddenSize13, HiddenSize14, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize15< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9,
                                int HiddenSize10, int HiddenSize11, int HiddenSize12, int HiddenSize13, int HiddenSize14, int HiddenSize15, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize7<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, HiddenSize12, HiddenSize13, HiddenSize14, HiddenSize15, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

public struct SharedMemorySize16< T, TargetEnum Target, ExecutionMode ExeMode, int SubgroupSize, int SubgroupCount,
                                int InputSize, int HiddenSize1, int HiddenSize2, int HiddenSize3, int HiddenSize4,
                                int HiddenSize5, int HiddenSize6, int HiddenSize7, int HiddenSize8, int HiddenSize9,
                                int HiddenSize10, int HiddenSize11, int HiddenSize12, int HiddenSize13, int HiddenSize14, int HiddenSize15, int HiddenSize16, int OutputSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    internal static const int a = SharedMemorySize15<T, Target, ExeMode, SubgroupSize, SubgroupCount, InputSize, HiddenSize1, HiddenSize2, HiddenSize3, HiddenSize4, HiddenSize5, HiddenSize6, HiddenSize7, HiddenSize8, HiddenSize9, HiddenSize10, HiddenSize11, HiddenSize12, HiddenSize13, HiddenSize14, HiddenSize15, HiddenSize16>.SharedMemSizeInBytes;
    internal static const int b = SharedMemorySize0<T, Target, ExeMode, SubgroupSize, SubgroupCount, HiddenSize16, OutputSize>.SharedMemSizeInBytes;

    public static const uint SharedMemSizeInBytes = Max(a, b);
}

// We should implement First/Rest syntax to something like this.
// SharedMemorySize<int In, each int HiddenSize, int OutputSize>
// {
//     static const uint SharedMemSizeInBytes =
//         max(SharedMemorySize<In, First HiddenSize>, SharedMemorySize<each Rest HiddenSize, OutputSize>);
// }

// Slang doesn't support generic overloading, therefore we cannot provide the pre-defined generics that adds different number of HiddenSize.
#if 0
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

internal typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

internal interface ISharedMemoryPool
{
    static SPtr<uint4> getPointer();
}

struct SharedMemoryPool<int Bytes> : ISharedMemoryPool
{
    internal static groupshared uint4 data[Bytes / sizeof(uint4)];
    internal static SPtr<uint4> getPointer()
    {
        return __getAddress(data[0]);
    }
}
