implementing neural;


namespace neural_unittest
{

// Full backward pass using TiledMMAHelper.backward() dispatcher:
//   CUDA:   TiledMMACuda (WaveMatrix register ops)
//   Vulkan: TiledMMAVulkan (WaveMatrix + shmem)
//
// InputSize and OutputSize must be multiples of 16 (tiled layout requirement).

DType[InputSize]
    testTiledBackwardImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount>
    (DType[InputSize] inputData,
     DType[OutputSize] dOutData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     RWStructuredBuffer<DType>.Handle weightGradBuffer,
     RWStructuredBuffer<DType>.Handle biasGradBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    typealias InputVec = WaveTangledVector<DType, ShMemSize, InputSize, 32, SubgroupCount>;
    typealias OutputVec = WaveTangledVector<DType, ShMemSize, OutputSize, 32, SubgroupCount>;
    typealias MMA = TiledMMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, true, SubgroupCount>;

    InputVec input = InputVec(inputData);
    OutputVec dOut = OutputVec(dOutData);

    typealias GradAddress = Address.Differential;
    let dInput = MMA.backward<Address, OutputVec.Differential, true>(
        input, bit_cast<OutputVec.Differential>(dOut), Address(weightBuffer),
        bit_cast<GradAddress>(Address(weightGradBuffer)),
        bit_cast<GradAddress>(Address(biasGradBuffer)));

    DType result[InputSize];
    for (int i = 0; i < InputSize; i++)
        result[i] = dInput[i];
    return result;
}

public DType[InputSize]
    testTiledBackward<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (DType[InputSize] inputData,
     DType[OutputSize] dOutData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     RWStructuredBuffer<DType>.Handle weightGradBuffer,
     RWStructuredBuffer<DType>.Handle biasGradBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        return testTiledBackwardImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount>(
            inputData, dOutData, weightBuffer, weightGradBuffer, biasGradBuffer);
    case spirv:
        return testTiledBackwardImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount>(
            inputData, dOutData, weightBuffer, weightGradBuffer, biasGradBuffer);
    }
}

} // namespace neural_unittest
