implementing neural;


namespace neural_unittest
{

void testBiasSumReduceImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        int InArraySize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount>
    (uint tid,
     __constref DType[InArraySize] dOut,
     RWStructuredBuffer<DType>.Handle biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address biasAddress = Address(biasBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, false, SubgroupCount>;
    const int OutSize = MMA.Uint4AlignedM;

    DType dOutVector[OutSize] = {};
    for (int i = 0; i < OutputSize; i++)
        dOutVector[i] = dOut[i];

    uint subgroupId = tid / 32;
    MMA.sumReduceRows<DType, Address, DType[OutSize]>(0, dOutVector, subgroupId, biasAddress);
}

void testBiasSumReduceMetalImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (uint tid,
     __constref WaveTangledVector<DType, ShMemSize, OutputSize, 32, SubgroupCount> dOut,
     RWStructuredBuffer<DType>.Handle biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address biasAddress = Address(biasBuffer);

    typealias MMA = TiledMMAMetal<DType, InputSize, OutputSize, 32, TargetEnum.Metal, ShMemSize, false, SubgroupCount>;

    uint subgroupId = tid / 32;
    MMA.sumReduceRows<DType, Address, WaveTangledVector<DType, ShMemSize, OutputSize, 32, SubgroupCount>>(
        0, dOut, subgroupId, biasAddress);
}

public void testBiasSumReduce<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (uint tid,
     __constref WaveTangledVector<DType, ShMemSize, OutputSize, 32, SubgroupCount> dOut,
     RWStructuredBuffer<DType>.Handle biasBuffer)
        where DType.Differential == DType
{
    typealias DOutVec = WaveTangledVector<DType, ShMemSize, OutputSize, 32, SubgroupCount>;

    __target_switch
    {
    case cuda:
        testBiasSumReduceImpl<DType, InputSize, OutputSize, DOutVec.Uint4AlignedInputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount>(
            tid, dOut.data, biasBuffer);
    case spirv:
        testBiasSumReduceImpl<DType, InputSize, OutputSize, DOutVec.Uint4AlignedInputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount>(
            tid, dOut.data, biasBuffer);
    case metal:
        testBiasSumReduceMetalImpl<DType, InputSize, OutputSize, ShMemSize, SubgroupCount>(
            tid, dOut, biasBuffer);
    }
}

} // namespace neural_unittest
