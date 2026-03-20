// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#include "common-def.slang"

// Perf profiling toggles. Override with -DPERF_xxx=0 to disable individual
// backward components for decomposed benchmarking.
#ifndef PERF_MMA_TRANSPOSE_ACTIVE
#define PERF_MMA_TRANSPOSE_ACTIVE 1
#endif
#ifndef PERF_BIAS_REDUCE_ACTIVE
#define PERF_BIAS_REDUCE_ACTIVE 1
#endif
#ifndef PERF_OPA_ACTIVE
#define PERF_OPA_ACTIVE 1
#endif

public enum TargetEnum : uint32_t
{
    CUDA = 0,
    SPIR_V = 1,
}

public enum ExecutionMode : uint32_t
{
    Inference = 0,
    Training = 1,
}

[ForceInline]
internal uint getWaveId()
{
    __target_switch
    {
    case cuda:
        uint3 tid = cudaThreadIdx();
        uint3 blockDim = cudaBlockDim();
        uint flattenedTid = tid.x + tid.y * blockDim.x + tid.z * blockDim.x * blockDim.y;
        return flattenedTid / WaveGetLaneCount();
    case spirv:
        return spirv_asm {
                OpCapability GroupNonUniform;
                result:$$uint = OpLoad builtin(SubgroupId:uint);
            };
    }
}

[ForceInline]
public int getWaveCount()
{
    // Note, we always require the threads count is multiple of the subgroup size, therefore we don't need to round up the result.
    __stage_switch
    {
    case compute:
        __target_switch
        {
        case cuda:
            uint3 blockDim = cudaBlockDim();
            int warpsPerBlock = (blockDim.x * blockDim.y * blockDim.z) >> 5;
            return warpsPerBlock;
        case spirv:
            uint3 workGroupSize = WorkgroupSize();
            int subGroupSize = WaveGetLaneCount();
            return (workGroupSize.x * workGroupSize.y * workGroupSize.z) / subGroupSize;
        }
    default:
        // We need this because WorkgroupSize() call requires compute stage only.
        static_assert(false, "Only support compute stage");
        return 0;
    }
}

internal struct CoopMatShape<T, TargetEnum Target>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int ROW_A = 16;
    static const int COLUMN_A = 16;

    // TODO: Currently, we only support floating point data type, therefore, we can always use 16x16x16.
    // Once in the future we want to extend to support integer type, we can uncomment the following code.
    static const int ROW_B = 16; // (Target == TargetEnum.CUDA) ? 16 : (sizeof(T) == 1 ? 32 : 16);
    static const int COLUMN_B = 16;

    static const int ROW_C = 16;
    static const int COLUMN_C = 16;

    static const int SizeAInElements = ROW_A * COLUMN_A;
    static const int SizeBInElements = ROW_B * COLUMN_B;
    static const int SizeCInElements = ROW_C * COLUMN_C;

    // Because the vectorized load is along K dimension (row of A and column of B), so we need to check
    // if K is aligned with the vector size (ElementCountPerVector), we can use the vectorized load,
    // otherwise, we need to use the scalar load.
    // ElementCountPerVector is measured in T.
    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);

    // Since A and B can only be half type, ElementCountPerVector is fixed. However, C can be both half and float.
    static const uint ElementCountPerVectorMatC = sizeof(uint4) / sizeof(T);

    static const uint CoopMatASizeInVector = (ROW_A * COLUMN_A) / ElementCountPerVector;
    static const uint CoopMatBSizeInVector = (ROW_B * COLUMN_B) / ElementCountPerVector;
    static const uint CoopMatCSizeInVector = (ROW_C * COLUMN_C) / ElementCountPerVectorMatC;
}

VISIBILITY_LEVEL struct TileInfo<T, int M, int N, int K, TargetEnum Target, bool TransposeA = false>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias MatShape = CoopMatShape<T, Target>;

    static const bool IsAlignedVector = (K % MatShape.ElementCountPerVector) == 0;

    // For the Tile A, how many cooperative matrices in a row
    static const int NCoopMatRow = TransposeA ? (K + MatShape.COLUMN_A - 1) / MatShape.COLUMN_A :
                                                (M + MatShape.ROW_A - 1) / MatShape.ROW_A;

    // Notice that because of our specific workload, NTilesColumn is always warp-size (as long as it's more than half warp) or half warp (<= half warp).
    // For the Tile B, how many cooperative matrices in a column
    static const int NCoopMatColumn = (N + MatShape.COLUMN_B - 1) / MatShape.COLUMN_B;

    // Total number of cooperative matrices that consist the result of C = A * B
    static const int NCoopMat = NCoopMatRow * NCoopMatColumn;

    // When A * B, k is the shared dimension, and when A^T * B, m is the shared dimension.
    static const uint SharedDimensionSize = TransposeA ? Uint4AlignedM : Uint4AlignedK;

    // `Uint4AlignedK` is the K value that is aligned with the uint4 vector size. Because we always do the vectorized load
    // along K dimension, so we need to align the K value with the vector size.
    // Measured in T.
    VISIBILITY_LEVEL static const int Uint4AlignedK = ((K + (MatShape.ElementCountPerVector - 1)) / MatShape.ElementCountPerVector) * MatShape.ElementCountPerVector;
    VISIBILITY_LEVEL static const int Uint4AlignedM = ((M + (MatShape.ElementCountPerVector - 1)) / MatShape.ElementCountPerVector) * MatShape.ElementCountPerVector;

    // The height of tile A measure in T and uint4.
    static const uint HeightInElementsTileA = NCoopMatRow * MatShape.ROW_A;
    static const uint HeightInVectorTileA = HeightInElementsTileA / MatShape.ElementCountPerVector;

    // In order to achieve the high throughput, we will write data to shared memory in vectorized way.
    // e.g. if T is half, it's 2 bytes, so uint4 can store 8 elements.
    // Since the size in bytes of the tile in one row is T.COLUMN * sizeof(T),
    // the number of vectorized columns in one row of cooperative matrix A is T.COLUMN * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4.
    static const uint WidthInVectorTileA = MatShape.COLUMN_A / MatShape.ElementCountPerVector;

    // WidthInElementTileA is the number of elements in one row of the tile in original data type.
    static const uint WidthInElementTileA = MatShape.COLUMN_A;

    // Similarly, the number of vectorized rows in one column of cooperative matrix B is T.ROW * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    static const uint HeightInVectorTileB = MatShape.ROW_B / MatShape.ElementCountPerVector;

    // So HeightInElementTileB is the number of elements in one column of the tile, measured in T.
    static const uint HeightInElementTileB = HeightInVectorTileB * MatShape.ElementCountPerVector;

    // The height of tile B measure in T and uint4.
    static const int WidthInElementsTileB = NCoopMatColumn * MatShape.COLUMN_B;
    static const int WidthInVectorTileB = WidthInElementsTileB / MatShape.ElementCountPerVector;

    static const uint HeightInVectorTileC = MatShape.ROW_C / MatShape.ElementCountPerVectorMatC;
}

// A wrapper of matrix type to access each tile of the matrix
// Note: The reason we want to make SubgroupSize as a generic parameter instead of using WaveGetLaneCount() is that
//       we need the logic to be expand as much as possible during compile time. WaveGetLaneCount() is a runtime function,
//       so if we use it directly in our mma function, some of branches won't be eliminated during compile time.
[require(cooperative_matrix)]
VISIBILITY_LEVEL struct MMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias ShMemPool = SharedMemoryPool<ShMemSize>;
    typealias TileInfo = TileInfo<T, OutputSize, SubgroupSize, InputSize, Target, TransposeA>;
    typealias CMShape = CoopMatShape<T, Target>;
    typealias ShMemInfo = SharedMemoryUsage<T, Target, TransposeA ? ExecutionMode.Training : ExecutionMode.Inference,
                                                   InputSize, OutputSize, SubgroupSize>;

    static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;
    static const linalg.CoopMatMatrixLayout ColumnMajor = linalg.CoopMatMatrixLayout.ColumnMajor;

    // Weight matrix is in shape of M x K
    // Input vector will be collected by each thread in a sub-group to form a matrix in shape of K x N
    // Note, M, K, N are measured in T.
    // Since Cooperative matrix is only worked for a whole subgroup (warp), so we always use the SubgroupSize as the N value.
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    VISIBILITY_LEVEL static const int Uint4AlignedK = TileInfo.Uint4AlignedK;
    VISIBILITY_LEVEL static const int Uint4AlignedM = TileInfo.Uint4AlignedM;

    // NumLoadsEachThreadMatA is the number of loads each thread needs to perform.
    // N is the number of active threads in the subgroup.
    // For matrix A, each iteration we load a column of matrix tile of Matrix A, which is height * sizeof(elementType) x 16 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to do for half type is
    // (M * 32) / (N * 16) = M * 2 / N, and we just round up the result to the nearest integer.
    static const uint NumLoadsEachThreadMatA = (TileInfo.HeightInElementsTileA * 2 + N - 1) / N;

    static const uint NumStoresEachThreadMatA = (sizeof(T) == 2) ?
                                                    NumLoadsEachThreadMatA :
                                                    (TileInfo.HeightInElementsTileA * 4 + N - 1) / N;

    // OffsetPerThreadLoadMatA is the offset between each load.
    // The reason is same as above, since the whole subgroup can load N * 16 bytes, and the load is consecutive,
    // so the offset is the N * 16 bytes, and we measure it in uint4 units, so it's N.
    static const uint OffsetPerThreadLoadMatA = N;

    // A/B can only be half, so we don't need to use generic parameter here.
    typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, CMShape.ROW_A,    CMShape.COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, CMShape.COLUMN_B, CMShape.COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T,    MemoryScope.Subgroup, CMShape.ROW_C,    CMShape.COLUMN_C, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    [ForceInline]
    static uint flattenedIndex<int Stride>(uint2 coord)
    {
        if (TransposeA)
        {
            return coord.y + coord.x * Stride;
        }
        else
        {
            return coord.x + coord.y * Stride;
        }
    }

    [ForceInline]
    static uint2 vectorizedTileCoordToWeightCoord<int ElemCountPerVector>(uint2 coordInTile, int tileIndex)
    {
        uint2 coordOut;
        if (TransposeA)
        {
            coordOut.x = coordInTile.x + tileIndex * TileInfo.WidthInElementTileA;
            coordOut.y = coordInTile.y * ElemCountPerVector;
        }
        else
        {
            coordOut.x = coordInTile.x * ElemCountPerVector + tileIndex * TileInfo.WidthInElementTileA;
            coordOut.y = coordInTile.y;
        }
        return coordOut;
    }

    [ForceInline]
    static uint2 vectorizedTileIndexTo2DCoord<int HeightInVector, int WidthInVector>(uint indexInVectorizedTile)
    {
        uint2 coordOut;
        if (TransposeA)
        {
            coordOut.x = indexInVectorizedTile / HeightInVector;
            coordOut.y = indexInVectorizedTile % HeightInVector;
        }
        else
        {
            coordOut.x = indexInVectorizedTile % WidthInVector;
            coordOut.y = indexInVectorizedTile / WidthInVector;
        }
        return coordOut;
    }
    // Load a tiled column of the Matrix A into shared memory.
    // We load a column of matrix tile of Matrix A each time, because our iteration is over K dimension.
    // We require the Matrix A is stored as row major in the weight storage.
    // Also we load the tile into shared memory in row major.
    // For each workgroup (thread block), we only use the first Subgroup (warp) to load the data.
    // This is just balanced decision between simplicity and bank conflict. Given that the shared memory tile
    // is not that large, if the warp is full, then each thread only needs to load 2 twice, therefore there is
    // no need to use more than one warp. And more warp could also make the algorithm avoiding bank conflict more
    // complicated.
    // So implement as this for now. We can profiling the application later, if it turns out to be a bottleneck,
    // we can consider using more than one warp in the future.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void matALoadStore<U, Address, bool isStore = false>(
        uint sharedMemoryOffset,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        uint indexInVectorizedTile = WaveGetLaneIndex();

        // In load case, since we always load the matrix into MatA, so the tile must in half type, because that is
        // what MatA can only support.
        // However, in store, we store the MatC which could be float type, so we need to adjust the tile height in vector
        // and element count per vector.
        static const int elementCountPerVector = isStore ? CMShape.ElementCountPerVectorMatC : CMShape.ElementCountPerVector;
        static const int heightInVector = isStore ?
                                        TileInfo.HeightInVectorTileA * CMShape.ElementCountPerVector / CMShape.ElementCountPerVectorMatC :
                                        TileInfo.HeightInVectorTileA;
        static const int widthInVector = isStore ?
                                        TileInfo.WidthInVectorTileA * CMShape.ElementCountPerVector / CMShape.ElementCountPerVectorMatC :
                                        TileInfo.WidthInVectorTileA;
        static const int numLoadOrStorePerThread = isStore ? NumStoresEachThreadMatA : NumLoadsEachThreadMatA;

        // get the 2-D coordinate of the tile in the vectorized tile
        const uint2 coordInTile = vectorizedTileIndexTo2DCoord<heightInVector, widthInVector>(indexInVectorizedTile);

        // tx and ty are the 2-D coordinate of scalar tile.
        // row coordinate ty is the same between vectorized tile and original tile.
        // column coordinate tx needs to be scaled by ElementCountPerVector to get the actual column coordinate in original data type.
        uint2 coordInWeight = vectorizedTileCoordToWeightCoord<elementCountPerVector>(coordInTile, tileIndex);

        uint4 value;
        uint indexInWeight;

        for (uint i = 0; i < numLoadOrStorePerThread; i++)
        {
            if (coordInWeight.y >= TileInfo.HeightInElementsTileA)
                break;
            // Given the coordinate inside the tile and the tile index, we can get the index in weight matrix.
            // Note, we always treat row of matrix A as uint4 aligned, so always calculate 'indexInWeight' as if it's uint4 aligned.
            // though it might not. But readUint4() will handle the padding.
            // We only need to provide the actual boundary and the aligned boundary.
            indexInWeight = flattenedIndex<K>(coordInWeight);

            // Bounds check on global memory.
            bool isOutOfRange = TransposeA ?
                                    ((coordInWeight.y >= K) | (coordInWeight.x >= M)) :
                                    ((coordInWeight.y >= M) | (coordInWeight.x >= K));

            if (isStore == false)
            {
                if (isOutOfRange)
                {
                    // If the coordinate is out of range of Matrix A, just padding with 0.
                    value = uint4(0, 0, 0, 0);
                }
                else
                {
                    // Though the weight matrix is not necessarily aligned with the uint4 vector size, readUint4() will handle the padding
                    // if the reading is cross the boundary of the uint4 vector size.
                    // MatA can only be half type, so we can always read it as half type no matter what the data type of the weight matrix is.
                    value = weightAddress.readUint4<half, TileInfo.IsAlignedVector, K>(int(indexInWeight));
                }

                ShMemPool.data[sharedMemoryOffset + indexInVectorizedTile] = value;
            }
            else
            {
                // We don't need padding when store back to global memory, because the padding is only needed when construct the cooperative matrix.
                // So if there is out of range, we don't need to store anything.
                if (!isOutOfRange)
                {
                    value = ShMemPool.data[sharedMemoryOffset + indexInVectorizedTile];
                    weightAddress.writeUint4Atomic<T, TileInfo.IsAlignedVector, K>(int(indexInWeight), value);
                }
            }

            indexInVectorizedTile += OffsetPerThreadLoadMatA;

            // Given the index in the tile, get the 2-D coordinate in the matrix A.
            // Note that, when TransposeA is true, this is 2-D coordinate in the matrix A^T. We don't change the semantics of the 2-D coordinate
            // in matrix, but we change the way to calculate the 1-D index in the matrix. But we can still use the same function to get the 2-D coordinate.
            coordInWeight = vectorizedTileCoordToWeightCoord<elementCountPerVector>(
                                vectorizedTileIndexTo2DCoord<heightInVector, widthInVector>(indexInVectorizedTile),
                                tileIndex);
        }
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadShA<U, Address>(
        uint sharedMemoryOffset,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        matALoadStore<U, Address, false>(sharedMemoryOffset, tileIndex, weightAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeShA<U, Address>(
        uint sharedMemoryOffset,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        matALoadStore<U, Address, true>(sharedMemoryOffset, tileIndex, weightAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void vectorLoadStore<U, InputArray, AccessOp Op = AccessOp.READ>(
        uint sharedMemoryBOffset,
        int tileIndex,
        int subgroupIndex,
        inout InputArray inoutVector)
            where U : __BuiltinFloatingPointType
            where InputArray : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();

        // select the correct shared memory layout configurations.
        // In store case, we actually store the MatC, whose data type could not be `half`.
        const uint elementPerVector = Op != AccessOp.READ ? CMShape.ElementCountPerVectorMatC : CMShape.ElementCountPerVector;
        const uint heightInVectorTile = Op != AccessOp.READ ? TileInfo.HeightInVectorTileC : TileInfo.HeightInVectorTileB;

        // Use SharedMemSizeInVectorMatC (not MatB) for per-warp stride even when
        // loading MatB. This ensures each warp's B and C regions occupy the same
        // slot, so the MatC writeback never overlaps an adjacent warp's MatB data.
        // The shared memory pool is already sized for MatC-width per-warp regions.
        const uint sharedMemSizeInVector = ShMemInfo.SharedMemSizeInVectorMatC;

        uint sharedMemoryBSubgroupOffset = sharedMemoryBOffset + subgroupIndex * sharedMemSizeInVector;

        // start index in vectorized tile for current thread: x * column_stride
        const uint indexInVectorizedTile = laneId * heightInVectorTile;
        const uint yOffset = (tileIndex * heightInVectorTile) * elementPerVector;

        for (uint yInTile = 0; yInTile < heightInVectorTile; yInTile++)
        {
            const int startIndex = yInTile * elementPerVector + yOffset;

            // Bounds check on Y direction. If y coordinate out of the input vector length, just padding with 0.
            // No need to check the X direction, because the X direction is bound by the thread count, so any active
            // thread will definitely have its thread-local vector available.

            bool isOutOfRange;

            if (Op == AccessOp.READ)
            {
                // For vector load, we are performing MMA, so the input vector has length of K
                // in normal case, and M in transpose case.
                isOutOfRange = TransposeA ? (startIndex >= M) : (startIndex >= K);
                if (isOutOfRange)
                {
                    ShMemPool.data[sharedMemoryBSubgroupOffset + indexInVectorizedTile + yInTile] = uint4(0, 0, 0, 0);
                    continue;
                }

                // It's fine that we only use the aligned version of readUint4() here, because the inputVector is the internal data which we can always
                // create it as an aligned array.
                uint4 value;

                // Note we can only use half type for matrix A and B, the value must be packed by 8 half type elements.
                accessUint4Aligned<AccessOp.READ, half, U, InputArray>(inoutVector, startIndex, value);
                ShMemPool.data[sharedMemoryBSubgroupOffset + indexInVectorizedTile + yInTile] = value;
            }
            else
            {
                // for vector store, the length of output vector has length of M in normal case
                // and K in transpose case.
                isOutOfRange = TransposeA ? (startIndex >= K) : (startIndex >= M);

                // No padding needs for writing back to local vector.
                if (isOutOfRange)
                    return;

                // in store case, we always store MatC, so the data type is T.
                uint4 value = ShMemPool.data[sharedMemoryBSubgroupOffset + indexInVectorizedTile + yInTile];
                accessUint4Aligned<Op, T, U, InputArray>(inoutVector, startIndex, value);
            }
        }
# if 0
        // TODO: In most case (on AMD and NVIDIA GPU), N (SubgroupSize) >= AlignedN (aligned to tile width of matrix B), as
        //       subgroup size is usually larger than the tile width of matrix B.
        //       But in some corner cases (e.g. intel GPU), where the hardware only support very small subgroup size, N could be less than AlignedN.
        //       Not sure if we want to support this case!
        if (N < AlignedN)
        {
            // If the active thread count is less than the aligned N, we need to pad the remaining columns with 0.
            const uint numPaddingColumns = AlignedN - N;

            // The number of vectors to load == numPaddingColumns * HeightInVectorTileB.
            // The number of vectors to load per thread == (numPaddingColumns * HeightInVectorTileB / N).
            const uint numLoadsEachThreadMatB = (numPaddingColumns * HeightInVectorTileB + N - 1) / N;
            const uint offsetPerThreadLoadMatB = N;

            // Because we already load N columns, the starting index is N * HeightInVectorTileB.
            uint paddingOffset = sharedMemoryBSubgroupOffset + N * HeightInVectorTileB;

            uint index = laneId;
            for (uint i = 0; i < numLoadsEachThreadMatB; i++)
            {
                const uint xIndex = index / HeightInVectorTileB;
                if (xIndex >= numPaddingColumns)
                    break;
                ShMemPool.data[paddingOffset + index] = uint4(0, 0, 0, 0);
                index += offsetPerThreadLoadMatB;
            }
        }
# endif
    }

    // Load the input vector into shared memory. Each input vector is a column of the matrix B.
    // Since the input vector is loaded from thread local memory, one thread can only load one
    // column of the matrix B.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadVectorToShB<U, InputArray>(
        uint sharedMemoryBOffset,
        int tileIndex,
        int subgroupIndex,
        InputArray inputVector)
            where U : __BuiltinFloatingPointType
            where InputArray : IArrayAccessor<U>
    {
        vectorLoadStore<U, InputArray, AccessOp.READ>(sharedMemoryBOffset, tileIndex, subgroupIndex, inputVector);
    }

    // Load the input vector into shared memory. Each input vector is a column of the matrix B.
    // Since the input vector is loaded from thread local memory, one thread can only load one
    // column of the matrix B.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeVectorFromShB<U, InputArray, AccessOp Op = AccessOp.WRITE>(
        uint sharedMemoryBOffset,
        int tileIndex,
        int subgroupIndex,
        inout InputArray inoutVector)
            where U : __BuiltinFloatingPointType
            where InputArray : IArrayAccessor<U>
    {
        vectorLoadStore<U, InputArray, Op>(sharedMemoryBOffset, tileIndex, subgroupIndex, inoutVector);
    }

    // Read one complete vector into the shared memory. Different from the loadVectorToShB,
    // this function is used to load the input vector into the shared memory for the transpose case, where each
    // tile will not contain the all the vectors in a warp. Instead, only partial warp will load the complete vector
    // into the shared memory. Compared to the LoadSharedMemoryFromLocalVector, where it loads partial vectors by every thread
    // in a warp.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadVectorForOuterProduct<int BatchSize, int ArraySizeInVector, int TileStrideInVector, U, InputArray>(
        uint sharedMemoryOffset,
        int tileIndex,
        InputArray inputVector)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where InputArray : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();

        // Due the limitation of the size of shared memory, we cannot load all the vectors in a warp to the shared memory. Instead,
        // we can only load it in smaller batches (batch size is the WMMA tile in column or row).
        if ((laneId / BatchSize) != tileIndex)
            return;

        uint index = (laneId % BatchSize) * TileStrideInVector;

        [ForceUnroll]
        for (uint i = 0; i < TileStrideInVector; i++)
        {
            uint4 value;
            if (i >= ArraySizeInVector)
            {
                value = uint4(0, 0, 0, 0);
            }
            else if (sizeof(U) == 2)
            {
                value = inputVector.readUint4(i);
            }
            else
            {
                uint4 pack1 = inputVector.readUint4(i * 2);
                uint4 pack2 = inputVector.readUint4(i * 2 + 1);
                float4 floatValue1 = bit_cast<float4>(pack1);
                float4 floatValue2 = bit_cast<float4>(pack2);
                half4 halfValue1 = half4(floatValue1);
                half4 halfValue2 = half4(floatValue2);
                value = uint4(bit_cast<uint2>(halfValue1), bit_cast<uint2>(halfValue2));
            }
            ShMemPool.data[sharedMemoryOffset + index + i] = value;
        }
    }

    // Cross-warp reduction for ALL tiles at once.
    // Uses more shared memory (ROW * COLUMN * TileStride) but only 2 barriers per reduction step
    // instead of 2 * ROW barriers. For 128x128 with 8x8 tiles:
    //   - Row-by-row: 8 rows * 2 barriers = 16 barriers
    //   - All-at-once: 1 step * 2 barriers = 2 barriers
    // Shared memory required: ROW * COLUMN * TileStride uint4 = 8 * 8 * 32 * 16 = 32KB for 128x128.
    [require(cooperative_matrix, subgroup_basic)]
    internal static void sumReduceTilesAllAtOnce<int ROW, int COLUMN>(inout MatC matC[ROW*COLUMN], in int subgroupId, uint sharedMemoryOffset)
    {
        if (SubgroupCount <= 1)
            return;

        static const int ElementsPerLane = CMShape.SizeCInElements / N;
        static const int RegsPerLane = (ElementsPerLane * sizeof(T) + 3) / 4;
        static const int TileStride = (RegsPerLane * N * 4 + sizeof(uint4) - 1) / sizeof(uint4);
        static const int TotalTiles = ROW * COLUMN;

        for (int k = 1; k < SubgroupCount; k <<= 1)
        {
            uint subgroupOffset = (subgroupId / (k<<1)) * (TotalTiles * TileStride);
            uint base = sharedMemoryOffset + subgroupOffset;

            // Store ALL tiles at once
            if (subgroupId % (k<<1) == k)
            {
                [ForceUnroll]
                for (int i = 0; i < ROW; i++)
                {
                    [ForceUnroll]
                    for (int j = 0; j < COLUMN; j++)
                    {
                        matC[i * COLUMN + j].storeNative(ShMemPool.data, base + (i * COLUMN + j) * TileStride);
                    }
                }
            }

            GroupMemoryBarrierWithGroupSync();

            // Load and add ALL tiles at once
            if (subgroupId % (k<<1) == 0)
            {
                if (subgroupId != (SubgroupCount - 1))
                {
                    [ForceUnroll]
                    for (int i = 0; i < ROW; i++)
                    {
                        [ForceUnroll]
                        for (int j = 0; j < COLUMN; j++)
                        {
                            MatC other;
                            other.loadNative(ShMemPool.data, base + (i * COLUMN + j) * TileStride);
                            matC[i * COLUMN + j] = matC[i * COLUMN + j] + other;
                        }
                    }
                }
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    // Helper function: Cross-warp reduction and atomic write to global memory.
    // Reads from shared memory where each warp has written its partial sums,
    // reduces across warps, and atomically adds to the bias gradient.
    //
    // For half type: Uses vector<half, 2> atomics (maps to red.relaxed.gpu.global.add.noftz.f16x2 on CUDA)
    // For other types: Uses scalar atomicAdd
    //
    // @param sharedMemory Shared memory containing partial sums from each warp
    // @param subgroupId Current warp index
    // @param subgroupCount Total number of warps
    // @param biasAddress Pointer-like address for bias gradient
    // @param MPadded M rounded up to even (for half2 alignment)
    // @param ColsPacked Number of packed pairs per warp (MPadded / 2)
    [ForceInline]
    static void crossWarpReduceAndAtomicWrite<U, Address, int MPadded, int ColsPacked>(
        uint sharedMemoryOffset,
        int subgroupId,
        int subgroupCount,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
    {
        const uint laneId = WaveGetLaneIndex();
        const int NUM_VECTORS = subgroupCount * N;
        const int ColsPerThread = ColsPacked / NUM_VECTORS;
        const int NumIters = (ColsPacked % NUM_VECTORS) != 0 ? ColsPerThread + 1 : ColsPerThread;

        int packed_col_idx = subgroupId * N + laneId;

        if (sizeof(U) == 2)
        {
            // Half path: pack two half values into uint for efficient shared memory access
            // Uses vector<half, 2> atomic which maps to red.relaxed.gpu.global.add.noftz.f16x2 on CUDA

            for (int j = 0; j < NumIters; j++)
            {
                if (packed_col_idx < ColsPacked)
                {
                    vector<U, 2> sum_v2 = vector<U, 2>(U(0), U(0));

                    for (int i = 0; i < subgroupCount; i++)
                    {
                        sum_v2 = sum_v2 + ShMemPool.smemLoad<vector<U, 2>>(sharedMemoryOffset, packed_col_idx + i * ColsPacked);
                    }

                    int elemIdx0 = packed_col_idx * 2;
                    int elemIdx1 = elemIdx0 + 1;

                    if (elemIdx1 < M)
                    {
                        let addr = biasAddress.getOffset(elemIdx0);
                        addr.atomicAdd(0u, sum_v2);
                    }
                    else if (elemIdx0 < M)
                    {
                        biasAddress.atomicAdd(uint(elemIdx0), sum_v2[0]);
                    }
                }

                packed_col_idx += NUM_VECTORS;
            }
        }
        else
        {
            // Non-half path: use scalar atomicAdd (no native vector atomic support)
            // Read individual elements from uint4 shared memory via smemLoad.

            for (int j = 0; j < NumIters; j++)
            {
                if (packed_col_idx < ColsPacked)
                {
                    int elemIdx0 = packed_col_idx * 2;
                    int elemIdx1 = elemIdx0 + 1;

                    // Sum elements across all warps
                    U sum0 = U(0);
                    U sum1 = U(0);

                    for (int i = 0; i < subgroupCount; i++)
                    {
                        sum0 = sum0 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded + elemIdx0);
                        if (elemIdx1 < MPadded)
                            sum1 = sum1 + ShMemPool.smemLoad<U>(sharedMemoryOffset, i * MPadded + elemIdx1);
                    }

                    if (elemIdx0 < M)
                    {
                        biasAddress.atomicAdd(uint(elemIdx0), sum0);
                    }
                    if (elemIdx1 < M)
                    {
                        biasAddress.atomicAdd(uint(elemIdx1), sum1);
                    }
                }

                packed_col_idx += NUM_VECTORS;
            }
        }
    }

    // Scalar warp-level reduction using WaveMaskSum/WaveActiveSum.
    // Fallback for targets without cooperative_matrix support.
    [ForceInline]
    [require(cuda_spirv, subgroup_partitioned)]
    [require(cuda_spirv, sm_6_6)]
    internal static void sumReduceRowsScalar<U, Address, InArrayType>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();
        const int subgroupCount = SubgroupCount;

        static const int MPadded = (M + 1) & ~1;
        static const int ColsPacked = MPadded / 2;

        InArrayType localResult = {};
        if (sizeof(U) == 2)
        {
            [ForceUnroll]
            for (int i = 0; i < MPadded; i += 2)
            {
                vector<U, 2> packed = vector<U, 2>(dOut[i], dOut[i + 1]);
                vector<U, 2> reduced = WaveMaskSum(0xFFFFFFFFu, packed);
                localResult[i] = reduced[0];
                localResult[i + 1] = reduced[1];
            }
        }
        else
        {
            for (int i = 0; i < M; i++)
            {
                localResult[i] = WaveActiveSum(dOut[i]);
            }
        }

        if (sizeof(U) == 2)
        {
            for (int i = int(laneId) * 2; i < MPadded; i += N * 2)
            {
                vector<U, 2> v2 = vector<U, 2>(localResult[i], localResult[i + 1]);
                ShMemPool.smemStore(sharedMemoryOffset, subgroupId * ColsPacked + i / 2, v2);
            }
        }
        else
        {
            for (int i = laneId; i < MPadded; i += N)
            {
                U val = (i < M) ? localResult[i] : U(0);
                ShMemPool.smemStore(sharedMemoryOffset, subgroupId * MPadded + i, val);
            }
        }

        GroupMemoryBarrierWithGroupSync();

        crossWarpReduceAndAtomicWrite<U, Address, MPadded, ColsPacked>(
            sharedMemoryOffset, subgroupId, subgroupCount, biasAddress);
    }

    // MMA-based warp reduction for bias gradient.
    // Uses MMA: A=all-ones × B=dOut_data → C=column-sums across lanes.
    // This eliminates all shuffles and dramatically reduces instruction count.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    [require(cuda_spirv, sm_6_6)]
    internal static void sumReduceRowsMMA<U, Address, InArrayType>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();
        const int subgroupCount = SubgroupCount;

        static const int MPadded = (M + 1) & ~1;
        static const int ColsPacked = MPadded / 2;

        static const int NOutputTiles = (M + CMShape.COLUMN_B - 1) / CMShape.COLUMN_B;
        static const int HeightInVecB = CMShape.ROW_B / CMShape.ElementCountPerVector;
        static const int TileStrideVec = CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;
        static const int H2PerTile = CMShape.COLUMN_C / 2;

        // Shmem layout:
        // 1. Per-warp input vectors: SubgroupCount * N * HeightInVecB uint4s
        // 2. Per-warp matC temp areas: SubgroupCount * TileStrideVec * ROW_C uint4s
        // 3. Cross-warp reduction area (written after all tiles processed)
        uint warpVecBase = sharedMemoryOffset + subgroupId * N * HeightInVecB;
        
        // Temp area for matC store (one tile worth per warp to avoid race conditions)
        static const int TempAreaPerWarp = TileStrideVec * CMShape.ROW_C;
        static const int AllWarpsTempOffset = SubgroupCount * N * HeightInVecB;
        uint tempBase = sharedMemoryOffset + AllWarpsTempOffset + subgroupId * TempAreaPerWarp;
        
        // Cross-warp reduction area comes after all temp areas
        static const int CrossWarpOffset = AllWarpsTempOffset + SubgroupCount * TempAreaPerWarp;
        uint crossWarpBase = sharedMemoryOffset + CrossWarpOffset;

        static const int TotalUint4 = (M + CMShape.ElementCountPerVector - 1) / CMShape.ElementCountPerVector;

        [ForceUnroll]
        for (int tc = 0; tc < NOutputTiles; tc++)
        {
            // Step 1: Each lane writes its dOut elements to shmem as half-packed uint4.
            // MatB requires half data, so float inputs must be converted.
            uint laneOff = warpVecBase + laneId * HeightInVecB;
            [ForceUnroll]
            for (uint y = 0; y < HeightInVecB; y++)
            {
                int uint4Idx = tc * HeightInVecB + y;
                uint4 value;
                if (uint4Idx >= TotalUint4)
                {
                    value = uint4(0, 0, 0, 0);
                }
                else if (sizeof(U) == 2)
                {
                    value = dOut.readUint4(uint4Idx);
                }
                else
                {
                    uint4 pack1 = dOut.readUint4(uint4Idx * 2);
                    uint4 pack2 = dOut.readUint4(uint4Idx * 2 + 1);
                    float4 f1 = bit_cast<float4>(pack1);
                    float4 f2 = bit_cast<float4>(pack2);
                    half4 h1 = half4(f1);
                    half4 h2 = half4(f2);
                    value = uint4(bit_cast<uint2>(h1), bit_cast<uint2>(h2));
                }
                ShMemPool.data[laneOff + y] = value;
            }
            GroupMemoryBarrierWithWaveSync();

            // Step 2: MMA reduction. A=all-ones, B=dOut data (RowMajor: rows=lanes).
            // Two MMA ops for 2 groups of 16 lanes (K=16 each, 32 total).
            MatA matOnes;
            matOnes.fill(half(1));
            MatC matC;
            matC.fill(T(0));

            [ForceUnroll]
            for (int g = 0; g < 2; g++)
            {
                let matB = MatB.Load<RowMajor, uint4>(ShMemPool.data,
                    warpVecBase + g * 16 * HeightInVecB, HeightInVecB);
                matC = linalg.coopMatMulAdd<T, false, half, half, T,
                    MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                    >(matOnes, matB, matC);
            }

            // Step 3: Store matC to temp shmem, extract row 0 to Phase 3 format.
            // All rows of C are identical (A was all-ones).
            matC.Store<RowMajor, uint4>(ShMemPool.data, tempBase, TileStrideVec);
            GroupMemoryBarrierWithWaveSync();

            // Copy row 0 to cross-warp reduction layout.
            // All rows of C are identical, so row 0 has the reduced values.
            // Only copy the elements that fit within ColsPacked (for half) or MPadded (for float).
            if (sizeof(T) == 2)
            {
                int dstBase = tc * H2PerTile;
                int remainingH2 = ColsPacked - dstBase;
                int h2ToCopy = (remainingH2 < H2PerTile) ? remainingH2 : H2PerTile;
                for (int p = int(laneId); p < h2ToCopy; p += N)
                {
                    vector<U, 2> v2 = ShMemPool.smemLoad<vector<U, 2>>(tempBase, p);
                    ShMemPool.smemStore(crossWarpBase, subgroupId * ColsPacked + dstBase + p, v2);
                }
            }
            else if (sizeof(T) == 4)
            {
                // Float: row 0 is 16 floats. Read each float, write to cross-warp layout.
                // Only copy elements that fit within MPadded.
                int dstBase = tc * CMShape.COLUMN_C;
                int remainingElems = MPadded - dstBase;
                int elemsToCopy = (remainingElems < CMShape.COLUMN_C) ? remainingElems : CMShape.COLUMN_C;
                for (int p = int(laneId); p < elemsToCopy; p += N)
                {
                    T val = ShMemPool.smemLoad<T>(tempBase, p);
                    ShMemPool.smemStore(crossWarpBase, subgroupId * MPadded + dstBase + p, val);
                }
            }
        }

        GroupMemoryBarrierWithGroupSync();

        // Phase 3: Cross-warp reduction and atomic write.
        crossWarpReduceAndAtomicWrite<U, Address, MPadded, ColsPacked>(
            crossWarpBase, subgroupId, subgroupCount, biasAddress);
    }

    // Public dispatcher: uses MMA path for CUDA (best performance),
    // falls back to scalar path for other targets (Vulkan, etc.)
    [ForceInline]
    [require(cuda_spirv, subgroup_partitioned)]
    [require(cuda_spirv, sm_6_6)]
    VISIBILITY_LEVEL static void sumReduceRows<U, Address, InArrayType>(
        uint sharedMemoryOffset,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        __target_switch
        {
        case cuda:
            sumReduceRowsMMA<U, Address, InArrayType>(sharedMemoryOffset, dOut, subgroupId, biasAddress);
        default:
            sumReduceRowsScalar<U, Address, InArrayType>(sharedMemoryOffset, dOut, subgroupId, biasAddress);
        }
    }

    // Load ALL lanes' vectors to shmem in one shot (no idle lanes).
    // All N lanes write simultaneously; lane i writes at offset i * TileStrideInVector.
    // Requires N * TileStrideInVector uint4 per warp (2x the per-batch allocation).
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadAllVectorsForOuterProduct<int ArraySizeInVector, int TileStrideInVector, U, InputArray>(
        uint sharedMemoryOffset,
        InputArray inputVector)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where InputArray : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();
        uint index = laneId * TileStrideInVector;

        [ForceUnroll]
        for (uint i = 0; i < TileStrideInVector; i++)
        {
            uint4 value;
            if (i >= ArraySizeInVector)
            {
                value = uint4(0, 0, 0, 0);
            }
            else if (sizeof(U) == 2)
            {
                value = inputVector.readUint4(i);
            }
            else
            {
                uint4 pack1 = inputVector.readUint4(i * 2);
                uint4 pack2 = inputVector.readUint4(i * 2 + 1);
                float4 floatValue1 = bit_cast<float4>(pack1);
                float4 floatValue2 = bit_cast<float4>(pack2);
                half4 halfValue1 = half4(floatValue1);
                half4 halfValue2 = half4(floatValue2);
                value = uint4(bit_cast<uint2>(halfValue1), bit_cast<uint2>(halfValue2));
            }
            ShMemPool.data[sharedMemoryOffset + index + i] = value;
        }
    }

    // Load vectors to shmem and compute the outer product via CoopMat MMA.
    // Shared by both linear and tiled layout outerProductAccumulate paths.
    //
    // All N lanes write their vectors simultaneously (no idle lanes), requiring
    // N * StrideInVectorTile shmem per warp instead of BatchSize * StrideInVectorTile.
    // This eliminates one barrier per k-iteration at the cost of 2x vector shmem.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void outerProduct<int CoopMatCRows, int CoopMatCColumns, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        uint sharedMemoryAOffset,
        uint sharedMemoryBOffset,
        uint subgroupIndex,
        InArrayTypeA inputVectorA,
        InArrayTypeB inputVectorB,
        inout MatC matC[CoopMatCRows * CoopMatCColumns])
            where TypeA : __BuiltinFloatingPointType
            where TypeA.Differential == TypeA
            where TypeB : __BuiltinFloatingPointType
            where TypeB.Differential == TypeB
            where InArrayTypeA : IArrayAccessor<TypeA>
            where InArrayTypeB : IArrayAccessor<TypeB>
    {
        static const int BatchSize = CMShape.COLUMN_A;
        static const int StrideInVectorTileA = CoopMatCRows * CMShape.ROW_A / CMShape.ElementCountPerVector;
        static const int StrideInVectorTileB = CoopMatCColumns * CMShape.COLUMN_B / CMShape.ElementCountPerVector;

        // Per-warp shmem: N * Stride (holds ALL lanes' vectors at once)
        uint warpLocalPtrAOffset = sharedMemoryAOffset + subgroupIndex * StrideInVectorTileA * N;
        uint warpLocalPtrBOffset = sharedMemoryBOffset + subgroupIndex * StrideInVectorTileB * N;

        // All 32 lanes write simultaneously — one load, one barrier
        loadAllVectorsForOuterProduct<TileInfo.Uint4AlignedM / CMShape.ElementCountPerVector, StrideInVectorTileA, TypeA, InArrayTypeA>(warpLocalPtrAOffset, inputVectorA);
        loadAllVectorsForOuterProduct<TileInfo.Uint4AlignedK / CMShape.ElementCountPerVector, StrideInVectorTileB, TypeB, InArrayTypeB>(warpLocalPtrBOffset, inputVectorB);
        GroupMemoryBarrierWithWaveSync();

        // MMA loop over k-dimension tiles; each tile reads from its region in the pre-loaded shmem
        static const int NumKTiles = N / CMShape.COLUMN_A;

        [ForceUnroll]
        for (uint k = 0; k < NumKTiles; k++)
        {
            uint tileBaseA = warpLocalPtrAOffset + k * BatchSize * StrideInVectorTileA;
            uint tileBaseB = warpLocalPtrBOffset + k * BatchSize * StrideInVectorTileB;

            MatA matA[CoopMatCRows];
            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
                matA[i] = MatA.Load<ColumnMajor, uint4>(ShMemPool.data, tileBaseA + i * CMShape.ROW_A / CMShape.ElementCountPerVector, StrideInVectorTileA);

            [ForceUnroll]
            for (uint j = 0; j < CoopMatCColumns; j++)
            {
                let matB = MatB.Load<RowMajor, uint4>(ShMemPool.data, tileBaseB + j * CMShape.COLUMN_B / CMShape.ElementCountPerVector, StrideInVectorTileB);

                for (uint i = 0; i < CoopMatCRows; i++)
                {
                    int index = i * CoopMatCColumns + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }
    }

    // Concrete outer product using WaveTangledVector's loadToSharedMemoryForOuterProduct.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    [require(spvAtomicFloat16AddEXT)]
    VISIBILITY_LEVEL static void outerProductAccumulate<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        uint sharedMemoryAOffset,
        uint sharedMemoryBOffset,
        InArrayTypeA inputVectorA,
        InArrayTypeB inputVectorB,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
            where TypeA : __BuiltinFloatingPointType
            where TypeA.Differential == TypeA
            where TypeB : __BuiltinFloatingPointType
            where TypeB.Differential == TypeB
            where InArrayTypeA : IArrayAccessor<TypeA>
            where InArrayTypeB : IArrayAccessor<TypeB>
    {
        const uint subgroupIndex = getWaveId();

        static const int CoopMatCRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
        static const int CoopMatCColumns = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
        static const int StrideInVectorTileC = CoopMatCColumns * CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;

        MatC matC[CoopMatCRows * CoopMatCColumns];

        [ForceUnroll]
        for (int i = 0; i < CoopMatCRows * CoopMatCColumns; i++)
            matC[i].fill(T(0.0f));

        outerProduct<CoopMatCRows, CoopMatCColumns, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
            sharedMemoryAOffset, sharedMemoryBOffset, subgroupIndex, inputVectorA, inputVectorB, matC);

        GroupMemoryBarrierWithGroupSync();
        // Use all-at-once reduction: 2 barriers instead of 2*CoopMatCRows barriers.
        // Start at offset 0 since the vector data is no longer needed after outerProduct.
        sumReduceTilesAllAtOnce<CoopMatCRows, CoopMatCColumns>(matC, subgroupIndex, 0);

        if (subgroupIndex == 0)
        {
            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                for (uint j = 0; j < CoopMatCColumns; j++)
                {
                    matC[i * CoopMatCColumns + j].Store<RowMajor, uint4>(
                        ShMemPool.data,
                        sharedMemoryBOffset + j * CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC,
                        (uint)StrideInVectorTileC);
                }

                // We will leverage the fact that the result the outerproduct is the same shape as matrix A. But, the tile organization is different.
                // In forward pass, the tile in the shared memory is in shape M x T_ROW_A, which is one column of cooperative matrices.
                // while the result here is in shape T_ROW_A x K, which is one row of cooperative matrices.
                // So we can use transposed version of LoadShA/StoreShA to store the result. Because in transpose version, the major-ness is along
                // K dimension. Note that column major in transpose case is just the row major in non-transpose case.

                typealias MMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, true, SubgroupCount>;
                MMA.storeShA<U, Address>(sharedMemoryBOffset, i, weightAddress);

                // Wait until all threads in the warp finish reading the shared memory and storing to the global memory.
                GroupMemoryBarrierWithWaveSync();
            }
        }

        AllMemoryBarrierWithGroupSync();
    }

    // The workload is different from the large size matrix-matrix multiply, we actually perform the matrix-vector multiply
    //  for each thread, so the matrix-matrix multiply only needs to be performed for each warp (sub-group).
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static OutArrayType mma<U, Address, InArrayType, OutArrayType, V, bool Bias = false>(InArrayType inputVector,
        uint sharedMemoryAOffset,
        uint sharedMemoryBOffset,
        uint sharedMemoryCOffset,
        Address weightAddress,
        Optional<Address> biasAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
            where V : __BuiltinFloatingPointType
            where V.Differential == V
            where InArrayType : IArrayAccessor<V>
            where OutArrayType : IArrayAccessor<V>
    {
        uint ptrAOffset = sharedMemoryAOffset;
        uint ptrBOffset = sharedMemoryBOffset;
        uint ptrCOffset = sharedMemoryCOffset;
        const uint subgroupIndex = getWaveId();

        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        MatC matC[TileInfo.NCoopMat];
        for (int i = 0; i < TileInfo.NCoopMat; i++)
        {
            matC[i].fill(T(0));
        }

        OutArrayType outputVector;
        if (Bias)
        {
            for (int i = 0; i < M; i++)
            {
                U bias = biasAddress.value[uint(i)];
                outputVector[i] = __realCast<V>(bias);
            }
        }

        for (int k = 0; k < TileInfo.SharedDimensionSize; k += CMShape.COLUMN_A)
        {
            uint tileIndex = k / CMShape.COLUMN_A;

            // Ensure all warps finished reading shared memory from the previous
            // tile iteration (or from the previous layer's output writeback in
            // fused multi-layer kernels) before overwriting with new tile data.
            GroupMemoryBarrierWithGroupSync();

            if (subgroupIndex == 0)
            {
                loadShA<U, Address>(ptrAOffset, tileIndex, weightAddress);
            }
            loadVectorToShB(ptrBOffset, tileIndex, subgroupIndex, inputVector);

            // Workgroup sync: ensures sharedMemoryA (written by first warp) is visible
            // to all warps, and sharedMemoryB writes are also complete.
            GroupMemoryBarrierWithGroupSync();

            // Math loop: This operator is executed by each warp (sub-group). shA is shared by all threads in the workgroup, while
            // shB is shared only by each subgroup, and each subgroup will have its own offset on shB.
            // For matB, each warp could only have 1 or 2 Tiles to load according whether this warp is less (1 tile) or more than half warp (2 tiles).
            MatA matA[TileInfo.NCoopMatRow];
            for (uint i = 0; i < TileInfo.NCoopMatRow; i ++)
            {
                if (TransposeA)
                {
                    // Really be careful the stride of tile A in the transpose case. The tile is column major, and the cooperative matrix is stacked in columns of the tile.
                    // So the stride is actually the height the tile. The offset between two cooperative matrices is just height of the cooperative matrix.
                    matA[i] = MatA.Load<ColumnMajor, uint4>(ShMemPool.data, ptrAOffset + i * CMShape.ROW_A/CMShape.ElementCountPerVector, TileInfo.HeightInVectorTileA);
                }
                else
                {
                    matA[i] = MatA.Load<RowMajor, uint4>(ShMemPool.data, ptrAOffset + i * CMShape.CoopMatASizeInVector, TileInfo.WidthInVectorTileA);
                }
            }

            for (uint j = 0; j < TileInfo.NCoopMatColumn; j ++)
            {
                // Use SharedMemSizeInVectorMatC (not MatB) for per-warp stride to match
                // the writeback layout, keeping each warp's B/C regions aligned.
                uint ptrPerWarpOffset = ptrBOffset + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatC;
                let matB = MatB.Load<ColumnMajor, uint4>(ShMemPool.data, ptrPerWarpOffset + j * CMShape.CoopMatBSizeInVector, TileInfo.HeightInVectorTileB);

                for (uint i = 0; i < TileInfo.NCoopMatRow; i ++)
                {
                    int index = i * TileInfo.NCoopMatColumn + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        // Write back the result to shared memory B. We store the result in column major because each column is
        // actually the output vector of the matrix-vector multiply, which is also a thread-local vector.

        // Get start offset of the shared memory for current warp.
        uint ptrPerWarpOffset = ptrCOffset + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatC;

        for (int i = 0; i < TileInfo.NCoopMatRow; i ++)
        {
            for (int j = 0; j < TileInfo.NCoopMatColumn; j ++)
            {
                int index = i * TileInfo.NCoopMatColumn + j;
                matC[index].Store<ColumnMajor, uint4>(ShMemPool.data, ptrPerWarpOffset + j * CMShape.CoopMatCSizeInVector,
                                TileInfo.HeightInVectorTileC);
            }

            if (Bias)
            {
                storeVectorFromShB<V, OutArrayType, AccessOp.ACCUMULATE>(ptrCOffset, i, subgroupIndex, outputVector);
            }
            else
            {
                storeVectorFromShB<V, OutArrayType, AccessOp.WRITE>(ptrCOffset, i, subgroupIndex, outputVector);
            }

            // wait until all threads in the warp finish reading shared memory and storing to local vector.
            GroupMemoryBarrierWithWaveSync();
        }

        return outputVector;
    }
}

// Cooperative matrix is only supported by CUDA and SPIR-V
// WaveTangledVector is a vector type that emulates the Cooperative Vector type by using Cooperative Matrix feature which is
// supported by CUDA and SPIR-V.
[require(cooperative_matrix, subgroup_basic)]
public struct WaveTangledVector<T, ShMemSize : ISharedMemorySize, int N, int SubgroupSize, int SubgroupCount> : IVector<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = WaveTangledVector<T.Differential, ShMemSize, N, SubgroupSize, SubgroupCount>;

    public static const int Size = N;
    public typealias ShMemPool = SharedMemoryPool<ShMemSize>;

    private typealias DTypeMatC = half;

    // Since we always use half type for A and B, we want to follow the alignment requirement of half type.
    internal static const uint ElementCountPerVector = sizeof(uint4) / sizeof(DTypeMatC);
    internal static const int Uint4AlignedInputSize = ((N + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    [DerivativeMember(Differential.data)]
    internal T[Uint4AlignedInputSize] data;
    public int getCount() { return N; }

    public __init() { data = {}; }

    public __init(T value)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = value;
        cleanPaddingData();
    }

    public __init(T[Size] inData)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = inData[i];
        cleanPaddingData();
    }

    internal __init(T[Uint4AlignedInputSize] inData)
    {
        this.data = inData;
    }

    public __init(This other) { this.data = other.data; }

    public __init<InputArray : IArray<T>>(InputArray inData)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = inData[i];
        cleanPaddingData();
    }

    [ForceInline]
    [mutating]
    internal void cleanPaddingData()
    {
        const int elementCountPerUint4 = sizeof(uint4) / sizeof(DTypeMatC);
        const int alignedSize = ((N + (elementCountPerUint4 - 1)) / elementCountPerUint4) * elementCountPerUint4;
        for (int i = N; i < alignedSize; i++)
            this.data[i] = T(0.0f);
    }

    public __subscript(int index) -> T
    {
        [ForceInline]
        get { return this.data[index]; }

        [ForceInline]
        set { this.data[index] = newValue; }
    }

    [ForceInline]
    override internal uint4 readUint4(int uint4Index)
    {
        return data.readUint4(uint4Index);
    }

    [ForceInline]
    private OutputVector linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum Target, bool Bias>(
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T.Differential>
        where OutputVector : IVector<T>
        where Layout : IStorageLayout
    {
        if (Layout.Layout == LayoutType.Optimal)
        {
            typealias MMA = TiledMMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, false, SubgroupCount>;
            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;
            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedM;
            let outputArray = MMA.mma<T, Address, T[Uint4AlignedInputSize], T[AlignedOutSize], T, Bias
                                >( data, shA, shB, shC, weightAddress, biasAddress);
            return OutputVector(outputArray);
        }
        else
        {
            typealias MMA = MMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, false, SubgroupCount>;
            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;
            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedM;
            let outputArray = MMA.mma<T, Address, T[Uint4AlignedInputSize], T[AlignedOutSize], T, Bias
                                >( data, shA, shB, shC, weightAddress, biasAddress);
            return OutputVector(outputArray);
        }
    }

    [ForceInline]
    private static void linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum Target, bool Bias>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        Optional<DifferentialPtrPair<Address>> biasAddress,
        OutputVector.Differential doutput)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T.Differential>
        where OutputVector : IVector<T>
        where OutputVector.Differential : IVector<T.Differential>
        where Layout : IStorageLayout
    {
        if (Layout.Layout == LayoutType.Optimal)
        {
            typealias MMA = TiledMMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, true, SubgroupCount>;

            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;

            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedK;

#if PERF_MMA_TRANSPOSE_ACTIVE
            // dIn = W^T * dOut (tiled layout)
            let outArray = MMA.mma<T, Address, OutputVector.Differential, T[AlignedOutSize].Differential, T.Differential, false
                            >( doutput, shA, shB, shC, dWeightAddress.p, none);
            This.Differential dInput = This.Differential(outArray);
            dthis = DifferentialPair<This>(dthis.p, dInput);

            GroupMemoryBarrierWithGroupSync();
#endif

#if PERF_BIAS_REDUCE_ACTIVE
            if (Bias)
            {
                const int subgroupIndex = getWaveId();
                MMA.sumReduceRows<T.Differential, Address.Differential, OutputVector.Differential
                    >(shA, doutput, subgroupIndex, biasAddress.value.d);
            }

            GroupMemoryBarrierWithGroupSync();
#endif

#if PERF_OPA_ACTIVE
            static const int _outerRows = (MMA.M + MMA.CMShape.ROW_A - 1) / MMA.CMShape.ROW_A;
            static const int _outerPerWarpA = _outerRows * MMA.CMShape.ROW_A / MMA.CMShape.ElementCountPerVector * SubgroupSize;
            uint shB_outer = shA + uint(SubgroupCount * _outerPerWarpA);

            // dW = dOut * input^T, stored in tiled layout
            MMA.outerProductAccumulate<T.Differential, Address.Differential,
                                       T.Differential, OutputVector.Differential,
                                       T, T[Uint4AlignedInputSize]
                                      >( shA, shB_outer, doutput, dthis.p.data, dWeightAddress.d);
#endif
        }
        else
        {
            typealias MMA = MMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, true, SubgroupCount>;

            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;

            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedK;

            // dIn = W^T * dOut (linear layout)
            let outArray = MMA.mma<T, Address, OutputVector.Differential, T[AlignedOutSize].Differential, T.Differential, false
                            >( doutput, shA, shB, shC, dWeightAddress.p, none);
            This.Differential dInput = This.Differential(outArray);
            dthis = DifferentialPair<This>(dthis.p, dInput);

            GroupMemoryBarrierWithGroupSync();

            static const int _outerRows = (MMA.M + MMA.CMShape.ROW_A - 1) / MMA.CMShape.ROW_A;
            static const int _outerPerWarpA = _outerRows * MMA.CMShape.ROW_A / MMA.CMShape.ElementCountPerVector * SubgroupSize;
            uint shB_outer = shA + uint(SubgroupCount * _outerPerWarpA);

            // dW = dOut * input^T, stored in linear layout
            MMA.outerProductAccumulate<T.Differential, Address.Differential,
                                       T.Differential, OutputVector.Differential,
                                       T, T[Uint4AlignedInputSize]
                                      >( shA, shB_outer, doutput, dthis.p.data, dWeightAddress.d);

            if (Bias)
            {
                const int subgroupIndex = getWaveId();
                MMA.sumReduceRows<T.Differential, Address.Differential, OutputVector.Differential
                    >(shA, doutput, subgroupIndex, biasAddress.value.d);
            }
        }
    }

    // Linear transformation without bias
    [ForceInline]
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        __target_switch
        {
        case cuda:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, false>(weightAddress, none);
        case spirv:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, false>(weightAddress, none);
        }
    }

    // Backward of linear transformation without bias
    [ForceInline]
    static void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T.Differential>
    {
        Optional<DifferentialPtrPair<Address>> biasAddress = {};
        __target_switch
        {
        case cuda:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, false>(dthis, dWeightAddress, biasAddress, doutput);
        case spirv:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, false>(dthis, dWeightAddress, biasAddress, doutput);
        }
    }

    [ForceInline]
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress, Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        __target_switch
        {
        case cuda:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, true>(weightAddress, biasAddress);
        case spirv:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, true>(weightAddress, biasAddress);
        }
    }

    // Backward of linear transformation with bias
    [ForceInline]
    static void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        DifferentialPtrPair<Address> dBiasAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T.Differential>
    {
        __target_switch
        {
        case cuda:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, true>(dthis, dWeightAddress, dBiasAddress, doutput);
        case spirv:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, true>(dthis, dWeightAddress, dBiasAddress, doutput);
        }
    }
}
