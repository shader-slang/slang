implementing neural;


namespace neural_unittest
{

// Full backward pass using TiledMMAHelper (OptimalLayout):
//   1. dIn = W^T * dOut         (transpose MMA)
//   2. dBias += sum(dOut)       (bias reduce)
//   3. dW += dOut * input^T     (outer product accumulate, tiled store)
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
    Address weightAddr = Address(weightBuffer);
    Address weightGradAddr = Address(weightGradBuffer);
    Address biasGradAddr = Address(biasGradBuffer);

    typealias MMA = TiledMMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, true, SubgroupCount>;

    uint shA = 0;
    uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
    uint shC = shB;

    const int AlignedM = MMA.TileInfoT.Uint4AlignedM;
    const int AlignedK = MMA.TileInfoT.Uint4AlignedK;

    DType dOutVector[AlignedM] = {};
    for (int i = 0; i < OutputSize; i++)
        dOutVector[i] = dOutData[i];

    DType inputVector[AlignedK] = {};
    for (int i = 0; i < InputSize; i++)
        inputVector[i] = inputData[i];

    // 1. dIn = W^T * dOut
    let dInArray = MMA.mma<DType, Address, DType[AlignedM], DType[AlignedK], DType, false>(
        dOutVector, shA, shB, shC, weightAddr, none);

    GroupMemoryBarrierWithGroupSync();

    // 2. dBias += sum(dOut)
    {
        const int subgroupIndex = getWaveId();
        MMA.sumReduceRows<DType, Address, DType[AlignedM]>(
            shA, dOutVector, subgroupIndex, biasGradAddr);
    }

    GroupMemoryBarrierWithGroupSync();

    // 3. dW += dOut * input^T (stored in tiled layout)
    static const int _outerRows = (MMA.M + MMA.CMShape.ROW_A - 1) / MMA.CMShape.ROW_A;
    static const int _outerPerWarpA = _outerRows * MMA.CMShape.ROW_A / MMA.CMShape.ElementCountPerVector * 32;
    uint shB_outer = shA + uint(SubgroupCount * _outerPerWarpA);

    MMA.outerProductAccumulate<DType, Address,
                               DType, DType[AlignedM],
                               DType, DType[AlignedK]>(
        shA, shB_outer, dOutVector, inputVector, weightGradAddr);

    DType result[InputSize];
    for (int i = 0; i < InputSize; i++)
        result[i] = dInArray[i];
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
