implementing neural;


namespace neural_unittest
{

void testBiasSumReduceImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount>
    (uint tid,
     DType[OutputSize] dOut,
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

public void testBiasSumReduce<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (uint tid,
     DType[OutputSize] dOut,
     RWStructuredBuffer<DType>.Handle biasBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        testBiasSumReduceImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount>(tid, dOut, biasBuffer);
    case spirv:
        testBiasSumReduceImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount>(tid, dOut, biasBuffer);
    }
}

} // namespace neural_unittest
