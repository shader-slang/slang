implementing neural;


namespace neural_unittest
{

DType[TransposeA ? InputSize : OutputSize]
    testMatVecMulImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (uint tid,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(weightBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, TransposeA, SubgroupCount>;
    const int InSize = TransposeA ? MMA.Uint4AlignedM : MMA.Uint4AlignedK;
    const int OutSize = TransposeA ? MMA.Uint4AlignedK : MMA.Uint4AlignedM;
    const int FillSize = TransposeA ? OutputSize : InputSize;
    const int ResultSize = TransposeA ? InputSize : OutputSize;

    DType inputVector[InSize] = {};
    for (int i = 0; i < FillSize; i++)
        inputVector[i] = DType(tid + 1);

    DType alignedOutput[OutSize];
    if (Bias)
    {
        Address biasAddress = Address(biasBuffer.value);
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, true>(
            inputVector, 0, 256, 256, address, biasAddress);
    }
    else
    {
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, false>(
            inputVector, 0, 256, 256, address, none);
    }

    DType output[ResultSize];
    for (int i = 0; i < ResultSize; i++)
        output[i] = alignedOutput[i];
    return output;
}

public DType[TransposeA ? InputSize : OutputSize]
    testMatVecMul<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (uint tid,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        return testMatVecMulImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, Bias, SubgroupCount, TransposeA>(tid, weightBuffer, biasBuffer);
    case spirv:
        return testMatVecMulImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, Bias, SubgroupCount, TransposeA>(tid, weightBuffer, biasBuffer);
    }
}

// ---------------------------------------------------------------------------
// testTiledMatVecMul: same as testMatVecMul but uses TiledMMAHelper
// with PadToTile for dimensions.
// ---------------------------------------------------------------------------

DType[TransposeA ? InputSize : OutputSize]
    testTiledMatVecMulImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        int PaddedInputSize,
        int PaddedOutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (uint tid,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(weightBuffer);

    typealias MMA = TiledMMAHelper<DType, PaddedInputSize, PaddedOutputSize, 32, Target, ShMemSize, TransposeA, SubgroupCount>;
    const int InSize = TransposeA ? MMA.TileInfoT.Uint4AlignedM : MMA.TileInfoT.Uint4AlignedK;
    const int OutSize = TransposeA ? MMA.TileInfoT.Uint4AlignedK : MMA.TileInfoT.Uint4AlignedM;
    const int FillSize = TransposeA ? OutputSize : InputSize;
    const int ResultSize = TransposeA ? InputSize : OutputSize;

    DType inputVector[InSize] = {};
    for (int i = 0; i < FillSize; i++)
        inputVector[i] = DType(tid + 1);

    DType alignedOutput[OutSize];
    if (Bias)
    {
        Address biasAddress = Address(biasBuffer.value);
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, true>(
            inputVector, 0, 256, 256, address, biasAddress);
    }
    else
    {
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, false>(
            inputVector, 0, 256, 256, address, none);
    }

    DType output[ResultSize];
    for (int i = 0; i < ResultSize; i++)
        output[i] = alignedOutput[i];
    return output;
}

public DType[TransposeA ? InputSize : OutputSize]
    testTiledMatVecMul<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        int PaddedInputSize,
        int PaddedOutputSize,
        ShMemSize : ISharedMemorySize,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (uint tid,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        return testTiledMatVecMulImpl<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, ShMemSize, TargetEnum.CUDA, Bias, SubgroupCount, TransposeA>(tid, weightBuffer, biasBuffer);
    case spirv:
        return testTiledMatVecMulImpl<DType, InputSize, OutputSize, PaddedInputSize, PaddedOutputSize, ShMemSize, TargetEnum.SPIR_V, Bias, SubgroupCount, TransposeA>(tid, weightBuffer, biasBuffer);
    }
}

} // namespace neural_unittest
