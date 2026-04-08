implementing neural;


namespace neural_unittest
{

// ---------------------------------------------------------------------------
// testOuterProductAccumulate (linear layout)
// ---------------------------------------------------------------------------

void testOuterProductAccumulateImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount,
        int WorkgroupSize>
    (uint tid,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(outputBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, false, SubgroupCount>;
    const int OutSize = MMA.Uint4AlignedM;
    const int InSize = MMA.Uint4AlignedK;
    const int OffsetA = 0;
    const int OffsetB = 64 * 4 * SubgroupCount;

    DType dOutVector[OutSize] = {};
    DType inputVector[InSize] = {};

    float scaleFactor = 1.0f / float(WorkgroupSize);

    for (int i = 0; i < InputSize; i++)
        inputVector[i] = DType((i + 1) * (tid + 1) * scaleFactor);

    for (int i = 0; i < OutputSize; i++)
        dOutVector[i] = DType((OutputSize - i) * scaleFactor);

    MMA.outerProductAccumulate<DType, Address, DType, DType[OutSize], DType, DType[InSize]>(
        OffsetA, OffsetB, dOutVector, inputVector, address);
}

public void testOuterProductAccumulate<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount,
        int WorkgroupSize>
    (uint tid,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        testOuterProductAccumulateImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount, WorkgroupSize>(tid, outputBuffer);
    case spirv:
        testOuterProductAccumulateImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount, WorkgroupSize>(tid, outputBuffer);
    }
}

// ---------------------------------------------------------------------------
// testOuterProductAccumulateTiled (tiled layout)
// ---------------------------------------------------------------------------

void testOuterProductAccumulateTiledImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount,
        int WorkgroupSize>
    (uint tid,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(outputBuffer);

    typealias MMA = TiledMMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, false, SubgroupCount>;
    const int OutSize = MMA.TileInfoT.Uint4AlignedM;
    const int InSize = MMA.TileInfoT.Uint4AlignedK;
    const int OffsetA = 0;
    const int OffsetB = 64 * 4 * SubgroupCount;

    DType dOutVector[OutSize] = {};
    DType inputVector[InSize] = {};

    float scaleFactor = 1.0f / float(WorkgroupSize);

    for (int i = 0; i < InputSize; i++)
        inputVector[i] = DType((i + 1) * (tid + 1) * scaleFactor);

    for (int i = 0; i < OutputSize; i++)
        dOutVector[i] = DType((OutputSize - i) * scaleFactor);

    MMA.outerProductAccumulate<DType, Address, DType, DType[OutSize], DType, DType[InSize]>(
        OffsetA, OffsetB, dOutVector, inputVector, address);
}

public void testOuterProductAccumulateTiled<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount,
        int WorkgroupSize>
    (uint tid,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        testOuterProductAccumulateTiledImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount, WorkgroupSize>(tid, outputBuffer);
    case spirv:
        testOuterProductAccumulateTiledImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount, WorkgroupSize>(tid, outputBuffer);
    }
}

} // namespace neural_unittest
