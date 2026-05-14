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
        int SubgroupCount>
    (DType[InputSize] inputData,
     DType[OutputSize] dOutData,
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

    for (int i = 0; i < InputSize; i++)
        inputVector[i] = inputData[i];

    for (int i = 0; i < OutputSize; i++)
        dOutVector[i] = dOutData[i];

    MMA.outerProductAccumulate<DType, Address, DType, DType[OutSize], DType, DType[InSize]>(
        OffsetA, OffsetB, dOutVector, inputVector, address);
}

public void testOuterProductAccumulate<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (DType[InputSize] inputData,
     DType[OutputSize] dOutData,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    __target_switch
    {
    case cuda:
        testOuterProductAccumulateImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.CUDA, SubgroupCount>(inputData, dOutData, outputBuffer);
    case spirv:
        testOuterProductAccumulateImpl<DType, InputSize, OutputSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount>(inputData, dOutData, outputBuffer);
    }
}

// ---------------------------------------------------------------------------
// testOuterProductAccumulateTiled (tiled layout)
//
// Routes to TiledMMACuda.outerProduct (CUDA) and
// TiledMMAVulkan.outerProductAccumulate (Vulkan) which produce
// row-major tiled output layout.
//
// InputSize and OutputSize must be tile-aligned (multiples of 16).
// Callers with arbitrary sizes should pad via PadToTile before calling.
// ---------------------------------------------------------------------------

public void testOuterProductAccumulateTiled<
        DType : __BuiltinFloatingPointType,
        int InputSize,
        int OutputSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (DType[InputSize] inputData,
     DType[OutputSize] dOutData,
     RWStructuredBuffer<DType>.Handle outputBuffer)
        where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(outputBuffer);

    static const int N = 32;

    __target_switch
    {
    case cuda:
    {
        typealias Cuda = TiledMMACuda<DType, InputSize, OutputSize, N, TargetEnum.CUDA>;
        static const int PackedSizeOut = OutputSize / 2;
        static const int PackedSizeIn = InputSize / 2;

        Cuda.DOutMatrix dOutMat;
        {
            half h[OutputSize];
            for (int i = 0; i < OutputSize; i++)
                h[i] = __realCast<half>(dOutData[i]);
            let packed = bit_cast<uint[PackedSizeOut]>(h);
            dOutMat.fromArrayPacked<PackedSizeOut>(packed);
        }

        Cuda.InputMatrix inputMat;
        {
            half h[InputSize];
            for (int i = 0; i < InputSize; i++)
                h[i] = __realCast<half>(inputData[i]);
            let packed = bit_cast<uint[PackedSizeIn]>(h);
            inputMat.fromArrayPacked<PackedSizeIn>(packed);
        }

        Cuda.outerProduct<DType, Address, ShMemSize, SubgroupCount>(
            0u, dOutMat, inputMat, address);
    }
    case spirv:
    {
        typealias Vulkan = TiledMMAVulkan<DType, InputSize, OutputSize, N, TargetEnum.SPIR_V, ShMemSize, false, SubgroupCount>;

        static const int HalvesPerUint4 = sizeof(uint4) / sizeof(half);
        static const int TileWidthUint4 = 16 / HalvesPerUint4;
        static const int ShmemPerWarp = N * TileWidthUint4;
        uint perWarpBase = uint(getWaveId()) * uint(ShmemPerWarp);

        DType dOutArr[OutputSize];
        for (int i = 0; i < OutputSize; i++)
            dOutArr[i] = dOutData[i];

        MatrixA<OutputSize, N> dOutA;
        dOutA.fromVectorViaShMem<ShMemSize, DType, OutputSize, false>(dOutArr, perWarpBase);

        DType inputArr[InputSize];
        for (int i = 0; i < InputSize; i++)
            inputArr[i] = inputData[i];

        Vulkan.outerProductAccumulate<Address, InputSize>(dOutA, inputArr, address, 0u);
    }
    default:
        static_assert(false, "Unsupported target");
    }
}

} // namespace neural_unittest
