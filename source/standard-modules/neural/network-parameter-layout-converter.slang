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

internal int2 cudaNativeLayoutToRowMajor(int nativeIndex)
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

    int2 coord;
    coord.x = rowGroup * 8 + tidLocal * 2 + pair;
    coord.y = sub * 8 + gid;
    return coord;
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
    }
}


internal struct ConverterThreadScope
{
    uint index;
    uint stride;
}

internal ConverterThreadScope getConverterThreadScope()
{
    // Flatten the full 3-D dispatch domain. Each invocation owns
    // index + k * stride, where stride is the total number of launched threads.
    uint3 workgroupCount = WorkgroupCount();
    int3 workgroupSize = WorkgroupSize();
    uint3 dispatchSize = workgroupCount * uint3(workgroupSize);

    ConverterThreadScope scope;
    scope.index = __builtinDispatchThreadID.x +
                  __builtinDispatchThreadID.y * dispatchSize.x +
                  __builtinDispatchThreadID.z * dispatchSize.x * dispatchSize.y;
    scope.stride = dispatchSize.x * dispatchSize.y * dispatchSize.z;
    return scope;
}


internal int2 physicalWeightIndexToLogicalCoordForTarget<TargetEnum Target>(
    int inputSize,
    int outputSize,
    int physicalWeightIndex)
{
    static const int TileSize = 16;

    int paddedInputSize = align16(inputSize);
    int paddedOutputSize = align16(outputSize);
    int2 coord;

    if (Target == TargetEnum.CUDA)
    {
        int tileIndex = physicalWeightIndex / (TileSize * TileSize);
        int indexInTile = physicalWeightIndex - tileIndex * (TileSize * TileSize);
        int tileCols = paddedOutputSize / TileSize;
        int inputTile = tileIndex / tileCols;
        int outputTile = tileIndex - inputTile * tileCols;
        int2 tileCoord = cudaNativeLayoutToRowMajor(indexInTile);

        coord.x = outputTile * TileSize + tileCoord.y;
        coord.y = inputTile * TileSize + tileCoord.x;
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

        coord.x = outputTile * TileSize + outputInTile;
        coord.y = inputTile * TileSize + inputInTile;
    }
    else
    {
        coord.x = physicalWeightIndex / paddedInputSize;
        coord.y = physicalWeightIndex - coord.x * paddedInputSize;
    }

    return coord;
}

internal int2 physicalWeightIndexToLogicalCoord(
    int inputSize,
    int outputSize,
    int physicalWeightIndex)
{
    __target_switch
    {
    case cuda:
        return physicalWeightIndexToLogicalCoordForTarget<TargetEnum.CUDA>(
            inputSize, outputSize, physicalWeightIndex);
    case spirv:
        return physicalWeightIndexToLogicalCoordForTarget<TargetEnum.SPIR_V>(
            inputSize, outputSize, physicalWeightIndex);
    case metal:
        return physicalWeightIndexToLogicalCoordForTarget<TargetEnum.Metal>(
            inputSize, outputSize, physicalWeightIndex);
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
            int2 coord = physicalWeightIndexToLogicalCoord(
                inputSize,
                outputSize,
                layerElementIndex);
            int outputIndex = coord.x;
            int inputIndex = coord.y;
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

    // Each dispatch invocation owns a flattened physical-output element stream:
    // elementIndex = dispatchThreadId.x + k * totalDispatchThreadCount.
    [require(cuda, compute)]
    [require(metal, compute)]
    [require(spirv, spirv_1_0, compute)]
    public static void toOptimalLayout<PortableAddress, OptimalAddress>(
        PortableAddress portableLayoutStorage,
        OptimalAddress optimalLayoutStorage)
            where PortableAddress : IPointerLikeAddress<T>
            where OptimalAddress : IPointerLikeAddress<T>
    {
        uint elementCount = uint(ElementCountPhysical);
        let scope = getConverterThreadScope();

        for (uint elementIndex = scope.index;
             elementIndex < elementCount;
             elementIndex += scope.stride)
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

    // Each dispatch invocation owns a flattened logical-output element stream:
    // elementIndex = dispatchThreadId.x + k * totalDispatchThreadCount.
    [require(cuda, compute)]
    [require(metal, compute)]
    [require(spirv, spirv_1_0, compute)]
    public static void toPortableLayout<OptimalAddress, PortableAddress>(
        OptimalAddress optimalLayoutStorage,
        PortableAddress portableLayoutStorage)
            where OptimalAddress : IPointerLikeAddress<T>
            where PortableAddress : IPointerLikeAddress<T>
    {
        uint elementCount = uint(ElementCountLogical);
        let scope = getConverterThreadScope();

        for (uint elementIndex = scope.index;
             elementIndex < elementCount;
             elementIndex += scope.stride)
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
