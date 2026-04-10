implementing neural;


namespace neural_unittest
{

DType[TransposeA ? InputSize : OutputSize]
    testMatVecMulImpl<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        int InArraySize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (DType[InArraySize] inputData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(weightBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, TransposeA, SubgroupCount>;
    const int InSize = TransposeA ? MMA.Uint4AlignedM : MMA.Uint4AlignedK;
    const int OutSize = TransposeA ? MMA.Uint4AlignedK : MMA.Uint4AlignedM;
    const int ResultSize = TransposeA ? InputSize : OutputSize;

    uint shA = 0;
    uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
    uint shC = shB;

    DType inputVector[InSize] = {};
    for (int i = 0; i < InArraySize; i++)
        inputVector[i] = inputData[i];

    DType alignedOutput[OutSize];
    if (Bias)
    {
        Address biasAddress = Address(biasBuffer.value);
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, true>(
            inputVector, shA, shB, shC, address, biasAddress);
    }
    else
    {
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, false>(
            inputVector, shA, shB, shC, address, none);
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
        int InArraySize,
        ShMemSize : ISharedMemorySize,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (DType[InArraySize] inputData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        return testMatVecMulImpl<DType, InputSize, OutputSize, InArraySize, ShMemSize, TargetEnum.CUDA, Bias, SubgroupCount, TransposeA>(inputData, weightBuffer, biasBuffer);
    case spirv:
        return testMatVecMulImpl<DType, InputSize, OutputSize, InArraySize, ShMemSize, TargetEnum.SPIR_V, Bias, SubgroupCount, TransposeA>(inputData, weightBuffer, biasBuffer);
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
        int InArraySize,
        int PaddedInputSize,
        int PaddedOutputSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (DType[InArraySize] inputData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(weightBuffer);

    static const int K = PaddedInputSize;
    static const int M = PaddedOutputSize;
    static const int N = 32;
    const int ResultSize = TransposeA ? InputSize : OutputSize;

    if (TransposeA)
    {
        // mmaTranspose: input has M elements, result has K elements
        //   CUDA:   result(N,K) = input(N,M) × W^T(M,K)
        //   Vulkan: result(K,N) = W^T(K,M) × input(M,N)
        DType output[ResultSize];

        __target_switch
        {
        case cuda:
        {
            typealias Cuda = TiledMMACuda<DType, K, M, N, TargetEnum.CUDA>;

            Cuda.DOutMatrix inputMat;
            half inputHalf[M];
            for (int i = 0; i < InArraySize; i++)
                inputHalf[i] = __realCast<half>(inputData[i]);
            for (int i = InArraySize; i < M; i++)
                inputHalf[i] = half(0);
            let inputPacked = bit_cast<uint[M / 2]>(inputHalf);
            inputMat.fromArrayPacked<M / 2>(inputPacked);

            Cuda.BwdResultMatrix resultMat;
            Cuda.mmaTranspose<Address>(inputMat, address, resultMat);

            static const int PackedResultSize = K / (sizeof(uint) / sizeof(DType));
            uint resultPacked[PackedResultSize];
            resultMat.toArrayPacked<PackedResultSize>(resultPacked);
            let resultArr = bit_cast<DType[K]>(resultPacked);
            for (int i = 0; i < ResultSize; i++)
                output[i] = resultArr[i];
        }
        case spirv:
        {
            typealias Vulkan = TiledMMAVulkan<DType, K, M, N, TargetEnum.SPIR_V, ShMemSize, TransposeA, SubgroupCount>;

            DType inputArr[M];
            for (int i = 0; i < InArraySize; i++)
                inputArr[i] = inputData[i];
            for (int i = InArraySize; i < M; i++)
                inputArr[i] = DType(0);

            // Per-warp shmem offset. With tile-row-at-a-time, each warp needs
            // 32 × max(TILE/HalvesPerUint4, TILE/ElemPerUint4_T) uint4 per group.
            static const int HalfTileWidth = 16 / (sizeof(uint4) / sizeof(half));
            static const int DTileTileWidth = 16 / (sizeof(uint4) / sizeof(DType));
            static const int PerWarpUint4 = N * ((DTileTileWidth > HalfTileWidth) ? DTileTileWidth : HalfTileWidth);
            uint shBase = uint(getWaveId()) * uint(PerWarpUint4);

            MatrixB<M, N> inputB;
            inputB.fromVectorViaShMem<ShMemSize, DType, M, false>(inputArr, shBase);

            DType resultArr[K] = {};
            Vulkan.mmaTranspose<Address>(inputB, address, resultArr, shBase);
            for (int i = 0; i < ResultSize; i++)
                output[i] = resultArr[i];
        }
        }
        return output;
    }
    else
    {
        // mma (forward): input has K elements, result has M elements
        typealias MMA = TiledMMAHelper<DType, K, M, N, Target, ShMemSize, TransposeA, SubgroupCount>;
        typealias InputVec = WaveTangledVector<DType, ShMemSize, K, N, SubgroupCount>;

        InputVec inputVec;
        for (int i = 0; i < InArraySize; i++)
            inputVec.data[i] = inputData[i];

        let resultVec = Bias
            ? MMA.forward<Address, true>(inputVec, address, Address(biasBuffer.value))
            : MMA.forward<Address, false>(inputVec, address, none);

        DType output[ResultSize];
        for (int i = 0; i < ResultSize; i++)
            output[i] = resultVec[i];
        return output;
    }
}

public DType[TransposeA ? InputSize : OutputSize]
    testTiledMatVecMul<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        int InArraySize,
        int PaddedInputSize,
        int PaddedOutputSize,
        ShMemSize : ISharedMemorySize,
        bool Bias,
        int SubgroupCount,
        bool TransposeA>
    (DType[InArraySize] inputData,
     RWStructuredBuffer<DType>.Handle weightBuffer,
     Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        return testTiledMatVecMulImpl<DType, InputSize, OutputSize, InArraySize, PaddedInputSize, PaddedOutputSize, ShMemSize, TargetEnum.CUDA, Bias, SubgroupCount, TransposeA>(inputData, weightBuffer, biasBuffer);
    case spirv:
        return testTiledMatVecMulImpl<DType, InputSize, OutputSize, InArraySize, PaddedInputSize, PaddedOutputSize, ShMemSize, TargetEnum.SPIR_V, Bias, SubgroupCount, TransposeA>(inputData, weightBuffer, biasBuffer);
    }
}

} // namespace neural_unittest
