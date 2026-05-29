implementing neural;

// Converts a network parameter block between the user-facing portable layout and
// the target-facing optimal layout used by neural matrix-vector kernels.
//
// Portable layout for each layer is:
//   weight[OutputSize][InputSize] in row-major order, followed by bias[OutputSize]
//   only when that layer's bit is set in BiasMask.
//
// Optimal layout for each layer always reserves the same physical envelope:
//   weight[AlignedOutputSize][AlignedInputSize] followed by
//   bias[AlignedOutputSize], where both aligned sizes are rounded up to 16.
// The target only decides how logical weight coordinates are ordered inside
// that envelope at runtime:
//   CUDA: native MatrixB tile layout over (InputSize, OutputSize).
//   SPIR-V/Vulkan: 16x16 tiled row-major over (OutputSize, InputSize).
//   Metal/other targets: padded row-major over (OutputSize, InputSize).
//
// Metal currently only needs an 8x8-aligned row-major shape, but the shared
// 16x16 physical envelope is always at least as large and keeps network layer
// offsets target-independent. The portable layout stays exact to the user's
// expected per-layer bias choices.
//
// BiasMask uses one uint64_t bit per layer: bit 0 for D0 -> D1, bit 1 for D1 -> D2, etc.
// Example: NetworkParameterLayoutConverter<float, 0b01, 3, 5, 2>
// means the 3 -> 5 layer has bias, and the 5 -> 2 layer does not.


internal interface ILayoutSize<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int ElementCountLogical;
    static const int ElementCountPhysical;
    static const int BytesLogical;
    static const int BytesPhysical;
}

internal int align16(int value)
{
    // Cooperative-matrix paths consume 16x16 tiles. Keep Metal on the same
    // 16-element padding rule so a single physical size can describe all targets.
    return ((value + 15) / 16) * 16;
}

internal struct AlignedSize<int Size>
{
    static const int Value = ((Size + 15) / 16) * 16;
}

internal int rowMajorToCudaNativeLayout(int row, int col)
{
    // The CUDA MMA load path expects MatrixB in the same per-lane register order
    // used by the tiled MMA helper. This maps a row-major coordinate inside one
    // 16x16 logical tile to that native MatB element index.
    int rowGroup = row < 8 ? 0 : 1;
    int rowInGroup = row - 8 * rowGroup;
    int tidLocal = rowInGroup / 2;
    int pair = rowInGroup % 2;
    int sub = col < 8 ? 0 : 1;
    int gid = col - 8 * sub;
    int lane = gid * 4 + tidLocal;
    int localPos = sub * 4 + rowGroup * 2 + pair;
    return lane * 8 + localPos;
}

internal void cudaNativeLayoutToRowMajor(
    int nativeIndex,
    out int row,
    out int col)
{
    // Inverse of rowMajorToCudaNativeLayout(): recover the row/column coordinate
    // inside a native CUDA MatrixB 16x16 tile from the physical element index.
    int lane = nativeIndex / 8;
    int localPos = nativeIndex - lane * 8;
    int gid = lane / 4;
    int tidLocal = lane - gid * 4;
    int sub = localPos / 4;
    int localInSub = localPos - sub * 4;
    int rowGroup = localInSub / 2;
    int pair = localInSub - rowGroup * 2;

    row = rowGroup * 8 + tidLocal * 2 + pair;
    col = sub * 8 + gid;
}

internal struct LayerConverter<
    T,
    bool HasBias,
    int InputSize,
    int OutputSize>
    : ILayoutSize<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int PaddedInputSize = AlignedSize<InputSize>.Value;
    static const int PaddedOutputSize = AlignedSize<OutputSize>.Value;

    // Logical counts describe the portable user-facing layer block:
    // tightly packed row-major weights, followed by optional unpadded bias.
    static const int WeightElementCountLogical = InputSize * OutputSize;
    static const int BiasElementCountLogical = HasBias ? OutputSize : 0;
    static const int ElementCountLogical = WeightElementCountLogical + BiasElementCountLogical;
    static const int BytesLogical = ElementCountLogical * sizeof(T);

    // Physical counts describe the shared target-facing layer block. The byte
    // size is intentionally independent of the selected backend; only
    // physicalWeightOffset() changes the ordering of values inside the block.
    // The bias slot is reserved even when HasBias is false, so the optimal
    // layout does not need a special case per layer.
    static const int WeightElementCountPhysical = PaddedInputSize * PaddedOutputSize;
    static const int BiasElementCountPhysical = PaddedOutputSize;
    static const int ElementCountPhysical = WeightElementCountPhysical + BiasElementCountPhysical;
    static const int BytesPhysical = ElementCountPhysical * sizeof(T);

    __init() {}
}

internal struct ConverterEnd<T> : ILayoutSize<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int ElementCountLogical = 0;
    static const int ElementCountPhysical = 0;
    static const int BytesLogical = 0;
    static const int BytesPhysical = 0;
}

internal struct ConverterNode<
    T,
    uint64_t BiasMask,
    int InputSize,
    int OutputSize,
    let each RestDim : int>
    : ILayoutSize<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const bool HasBias = (BiasMask & 1ull) != 0ull;
    static const uint64_t RestBiasMask = BiasMask >> 1;

    typealias ThisLayer = LayerConverter<T, HasBias, InputSize, OutputSize>;
    typealias Tail = __packBranch(
        RestDim,
        ConverterEnd<T>,
        ConverterNode<T, RestBiasMask, OutputSize, __first(RestDim), __trimFirst(RestDim)>);

    // A network with dimensions <D0, D1, D2, ...> is encoded as one layer
    // D0 -> D1 followed by the converter for <D1, D2, ...>. These constants
    // describe the concatenated per-layer parameter blocks.
    static const int ElementCountLogical = ThisLayer.ElementCountLogical + Tail.ElementCountLogical;
    static const int ElementCountPhysical = ThisLayer.ElementCountPhysical + Tail.ElementCountPhysical;
    static const int BytesLogical = ElementCountLogical * sizeof(T);
    static const int BytesPhysical = ElementCountPhysical * sizeof(T);
}


internal int physicalWeightOffsetForTarget<TargetEnum Target>(
    int inputSize,
    int outputSize,
    int outputIndex,
    int inputIndex)
{
    static const int TileSize = 16;

    int paddedInputSize = align16(inputSize);
    int paddedOutputSize = align16(outputSize);
    int outputTile = outputIndex / TileSize;
    int inputTile = inputIndex / TileSize;
    int outputInTile = outputIndex - outputTile * TileSize;
    int inputInTile = inputIndex - inputTile * TileSize;

    if (Target == TargetEnum.CUDA)
    {
        int tileCols = paddedOutputSize / TileSize;
        int tileBase = (inputTile * tileCols + outputTile) * (TileSize * TileSize);
        return tileBase + rowMajorToCudaNativeLayout(inputInTile, outputInTile);
    }
    else if (Target == TargetEnum.SPIR_V)
    {
        int tileCols = paddedInputSize / TileSize;
        int tileBase = (outputTile * tileCols + inputTile) * (TileSize * TileSize);
        return tileBase + outputInTile * TileSize + inputInTile;
    }
    else
    {
        return outputIndex * paddedInputSize + inputIndex;
    }
}


internal int physicalWeightOffset(
    int inputSize,
    int outputSize,
    int outputIndex,
    int inputIndex)
{
    __target_switch
    {
    case cuda:
        return physicalWeightOffsetForTarget<TargetEnum.CUDA>(
            inputSize, outputSize, outputIndex, inputIndex);
    case spirv:
        return physicalWeightOffsetForTarget<TargetEnum.SPIR_V>(
            inputSize, outputSize, outputIndex, inputIndex);
    case metal:
        return physicalWeightOffsetForTarget<TargetEnum.Metal>(
            inputSize, outputSize, outputIndex, inputIndex);
    default:
        static_assert(false, "NetworkParameterLayoutConverter supports CUDA, SPIR-V, and Metal targets only");
        return 0;
    }
}



internal void physicalWeightIndexToLogicalCoordForTarget<TargetEnum Target>(
    int inputSize,
    int outputSize,
    int physicalWeightIndex,
    out int outputIndex,
    out int inputIndex)
{
    static const int TileSize = 16;

    int paddedInputSize = align16(inputSize);
    int paddedOutputSize = align16(outputSize);

    if (Target == TargetEnum.CUDA)
    {
        int tileIndex = physicalWeightIndex / (TileSize * TileSize);
        int indexInTile = physicalWeightIndex - tileIndex * (TileSize * TileSize);
        int tileCols = paddedOutputSize / TileSize;
        int inputTile = tileIndex / tileCols;
        int outputTile = tileIndex - inputTile * tileCols;
        int tileRow = 0;
        int tileCol = 0;
        cudaNativeLayoutToRowMajor(indexInTile, tileRow, tileCol);

        outputIndex = outputTile * TileSize + tileCol;
        inputIndex = inputTile * TileSize + tileRow;
    }
    else if (Target == TargetEnum.SPIR_V)
    {
        int tileIndex = physicalWeightIndex / (TileSize * TileSize);
        int indexInTile = physicalWeightIndex - tileIndex * (TileSize * TileSize);
        int tileCols = paddedInputSize / TileSize;
        int outputTile = tileIndex / tileCols;
        int inputTile = tileIndex - outputTile * tileCols;
        int outputInTile = indexInTile / TileSize;
        int inputInTile = indexInTile - outputInTile * TileSize;

        outputIndex = outputTile * TileSize + outputInTile;
        inputIndex = inputTile * TileSize + inputInTile;
    }
    else
    {
        outputIndex = physicalWeightIndex / paddedInputSize;
        inputIndex = physicalWeightIndex - outputIndex * paddedInputSize;
    }
}

internal void physicalWeightIndexToLogicalCoord(
    int inputSize,
    int outputSize,
    int physicalWeightIndex,
    out int outputIndex,
    out int inputIndex)
{
    __target_switch
    {
    case cuda:
        physicalWeightIndexToLogicalCoordForTarget<TargetEnum.CUDA>(
            inputSize, outputSize, physicalWeightIndex, outputIndex, inputIndex);
    case spirv:
        physicalWeightIndexToLogicalCoordForTarget<TargetEnum.SPIR_V>(
            inputSize, outputSize, physicalWeightIndex, outputIndex, inputIndex);
    case metal:
        physicalWeightIndexToLogicalCoordForTarget<TargetEnum.Metal>(
            inputSize, outputSize, physicalWeightIndex, outputIndex, inputIndex);
    default:
        static_assert(false, "NetworkParameterLayoutConverter supports CUDA, SPIR-V, and Metal targets only");
        outputIndex = 0;
        inputIndex = 0;
    }
}

internal void convertOneLayerToOptimalLayout<
    T,
    PortableAddress,
    OptimalAddress>(
    inout uint64_t biasMask,
    inout int inputSize,
    inout int logicalOffset,
    inout int physicalOffset,
    PortableAddress portableLayoutStorage,
    OptimalAddress optimalLayoutStorage,
    int outputSize,
    uint elementIndex)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
        where PortableAddress : IPointerLikeAddress<T>
        where OptimalAddress : IPointerLikeAddress<T>
{
    bool hasBias = (biasMask & 1ull) != 0ull;
    int weightElementCountLogical = inputSize * outputSize;
    int weightElementCountPhysical = align16(inputSize) * align16(outputSize);
    int elementCountPhysical = weightElementCountPhysical + align16(outputSize);
    PortableAddress portableLayer = portableLayoutStorage.getOffset(logicalOffset);
    OptimalAddress optimalLayer = optimalLayoutStorage.getOffset(physicalOffset);

    int layerElementIndex = int(elementIndex) - physicalOffset;
    if (layerElementIndex >= 0 && layerElementIndex < elementCountPhysical)
    {
        T value = T(0);

        if (layerElementIndex < weightElementCountPhysical)
        {
            int outputIndex = 0;
            int inputIndex = 0;
            physicalWeightIndexToLogicalCoord(
                inputSize,
                outputSize,
                layerElementIndex,
                outputIndex,
                inputIndex);
            if (outputIndex < outputSize && inputIndex < inputSize)
            {
                int srcOffset = outputIndex * inputSize + inputIndex;
                value = portableLayer[uint(srcOffset)];
            }
        }
        else
        {
            int biasIndex = layerElementIndex - weightElementCountPhysical;
            if (hasBias && biasIndex < outputSize)
                value = portableLayer[uint(weightElementCountLogical + biasIndex)];
        }

        optimalLayer[uint(layerElementIndex)] = value;
    }

    logicalOffset += weightElementCountLogical + (hasBias ? outputSize : 0);
    physicalOffset += elementCountPhysical;
    inputSize = outputSize;
    biasMask = biasMask >> 1;
}

internal void convertOneLayerToPortableLayout<
    T,
    OptimalAddress,
    PortableAddress>(
    inout uint64_t biasMask,
    inout int inputSize,
    inout int physicalOffset,
    inout int logicalOffset,
    OptimalAddress optimalLayoutStorage,
    PortableAddress portableLayoutStorage,
    int outputSize,
    uint elementIndex)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
        where OptimalAddress : IPointerLikeAddress<T>
        where PortableAddress : IPointerLikeAddress<T>
{
    bool hasBias = (biasMask & 1ull) != 0ull;
    int weightElementCountLogical = inputSize * outputSize;
    int weightElementCountPhysical = align16(inputSize) * align16(outputSize);
    int elementCountLogical = weightElementCountLogical + (hasBias ? outputSize : 0);
    int elementCountPhysical = weightElementCountPhysical + align16(outputSize);
    OptimalAddress optimalLayer = optimalLayoutStorage.getOffset(physicalOffset);
    PortableAddress portableLayer = portableLayoutStorage.getOffset(logicalOffset);

    int layerElementIndex = int(elementIndex) - logicalOffset;
    if (layerElementIndex >= 0 && layerElementIndex < elementCountLogical)
    {
        T value = T(0);

        if (layerElementIndex < weightElementCountLogical)
        {
            int outputIndex = layerElementIndex / inputSize;
            int inputIndex = layerElementIndex - outputIndex * inputSize;
            int srcOffset = physicalWeightOffset(
                inputSize,
                outputSize,
                outputIndex,
                inputIndex);
            value = optimalLayer[uint(srcOffset)];
        }
        else
        {
            int biasIndex = layerElementIndex - weightElementCountLogical;
            value = optimalLayer[uint(weightElementCountPhysical + biasIndex)];
        }

        portableLayer[uint(layerElementIndex)] = value;
    }

    physicalOffset += elementCountPhysical;
    logicalOffset += elementCountLogical;
    inputSize = outputSize;
    biasMask = biasMask >> 1;
}

internal void convertNetworkToOptimalLayout<
    T,
    uint64_t BiasMask,
    int InputSize,
    PortableAddress,
    OptimalAddress,
    let each OutDim : int>(
    PortableAddress portableLayoutStorage,
    OptimalAddress optimalLayoutStorage,
    uint elementIndex)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
        where PortableAddress : IPointerLikeAddress<T>
        where OptimalAddress : IPointerLikeAddress<T>
{
    uint64_t biasMask = BiasMask;
    int inputSize = InputSize;
    int logicalOffset = 0;
    int physicalOffset = 0;
    expand convertOneLayerToOptimalLayout<T, PortableAddress, OptimalAddress>(
        biasMask,
        inputSize,
        logicalOffset,
        physicalOffset,
        portableLayoutStorage,
        optimalLayoutStorage,
        each OutDim,
        elementIndex);
}

internal void convertNetworkToPortableLayout<
    T,
    uint64_t BiasMask,
    int InputSize,
    OptimalAddress,
    PortableAddress,
    let each OutDim : int>(
    OptimalAddress optimalLayoutStorage,
    PortableAddress portableLayoutStorage,
    uint elementIndex)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
        where OptimalAddress : IPointerLikeAddress<T>
        where PortableAddress : IPointerLikeAddress<T>
{
    uint64_t biasMask = BiasMask;
    int inputSize = InputSize;
    int physicalOffset = 0;
    int logicalOffset = 0;
    expand convertOneLayerToPortableLayout<T, OptimalAddress, PortableAddress>(
        biasMask,
        inputSize,
        physicalOffset,
        logicalOffset,
        optimalLayoutStorage,
        portableLayoutStorage,
        each OutDim,
        elementIndex);
}


public struct NetworkParameterLayoutConverter<
    T : __BuiltinFloatingPointType,
    uint64_t BiasMask,
    int InputSize,
    int OutputSize,
    let each RestDim : int>
        where T.Differential == T
{
    typealias Impl = ConverterNode<T, BiasMask, InputSize, OutputSize, RestDim>;

    public static const int ElementCountLogical = Impl.ElementCountLogical;
    public static const int ElementCountPhysical = Impl.ElementCountPhysical;
    public static const int BytesLogical = Impl.BytesLogical;
    public static const int BytesPhysical = Impl.BytesPhysical;

    // Each caller-provided thread owns part of the flattened physical-output
    // element stream: elementIndex = threadIndex + k * threadCount.
    [require(cuda, compute)]
    [require(metal, compute)]
    [require(spirv, spirv_1_0, compute)]
    public static void toOptimalLayout<PortableAddress, OptimalAddress>(
        PortableAddress portableLayoutStorage,
        OptimalAddress optimalLayoutStorage,
        uint threadIndex,
        uint threadCount)
            where PortableAddress : IPointerLikeAddress<T>
            where OptimalAddress : IPointerLikeAddress<T>
    {
        if (threadCount == 0)
            return;

        uint elementCount = uint(ElementCountPhysical);

        for (uint elementIndex = threadIndex;
             elementIndex < elementCount;
             elementIndex += threadCount)
        {
            convertNetworkToOptimalLayout<
                T,
                BiasMask,
                InputSize,
                PortableAddress,
                OptimalAddress,
                OutputSize,
                RestDim>(portableLayoutStorage, optimalLayoutStorage, elementIndex);
        }
    }

    // Each caller-provided thread owns part of the flattened logical-output
    // element stream: elementIndex = threadIndex + k * threadCount.
    [require(cuda, compute)]
    [require(metal, compute)]
    [require(spirv, spirv_1_0, compute)]
    public static void toPortableLayout<OptimalAddress, PortableAddress>(
        OptimalAddress optimalLayoutStorage,
        PortableAddress portableLayoutStorage,
        uint threadIndex,
        uint threadCount)
            where OptimalAddress : IPointerLikeAddress<T>
            where PortableAddress : IPointerLikeAddress<T>
    {
        if (threadCount == 0)
            return;

        uint elementCount = uint(ElementCountLogical);

        for (uint elementIndex = threadIndex;
             elementIndex < elementCount;
             elementIndex += threadCount)
        {
            convertNetworkToPortableLayout<
                T,
                BiasMask,
                InputSize,
                OptimalAddress,
                PortableAddress,
                OutputSize,
                RestDim>(optimalLayoutStorage, portableLayoutStorage, elementIndex);
        }
    }
}
