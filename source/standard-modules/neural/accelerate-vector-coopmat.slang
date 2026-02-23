// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#include "common-def.slang"

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

// We can't convert T* to uint4* on shared memory in slang, therefore, we will provide two versions of shared memory pointer
// and implement both of them, and document about the difference between them.
internal typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

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

struct TileInfo<T, int M, int N, int K, TargetEnum Target, bool TransposeA = false>
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
    static const int Uint4AlignedK = ((K + (MatShape.ElementCountPerVector - 1)) / MatShape.ElementCountPerVector) * MatShape.ElementCountPerVector;
    static const int Uint4AlignedM = ((M + (MatShape.ElementCountPerVector - 1)) / MatShape.ElementCountPerVector) * MatShape.ElementCountPerVector;

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
VISIBILITY_LEVEL struct MMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, bool TransposeA = false>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
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

    // TODO: This will always double the buffer usage, therefore we haven't really implemented the double buffer yet.
    static const bool EnableDoubleBuffer = false;

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
        SPtr<uint4> sharedMemory,
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

        [ForceUnroll]
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

                sharedMemory[indexInVectorizedTile] = value;
            }
            else
            {
                // We don't need padding when store back to global memory, because the padding is only needed when construct the cooperative matrix.
                // So if there is out of range, we don't need to store anything.
                if (!isOutOfRange)
                {
                    value = sharedMemory[indexInVectorizedTile];
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
        SPtr<uint4> sharedMemory,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        matALoadStore<U, Address, false>(sharedMemory, tileIndex, weightAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeShA<U, Address>(
        SPtr<uint4> sharedMemory,
        int tileIndex,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        matALoadStore<U, Address, true>(sharedMemory, tileIndex, weightAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    internal static void vectorLoadStore<U, InputArray, AccessOp Op = AccessOp.READ>(
        SPtr<uint4> sharedMemoryB,
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
        const uint sharedMemSizeInVector = Op != AccessOp.READ ? ShMemInfo.SharedMemSizeInVectorMatC : ShMemInfo.SharedMemSizeInVectorMatB;

        // In store case, we actually store the MatC. So the shared memory size could be different from MatB.
        SPtr<uint4> sharedMemoryBSubgroup = sharedMemoryB + subgroupIndex * sharedMemSizeInVector;

        // start index in vectorized tile for current thread: x * column_stride
        const uint indexInVectorizedTile = laneId * heightInVectorTile;
        const uint yOffset = (tileIndex * heightInVectorTile) * elementPerVector;

        [ForceUnroll]
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
                    sharedMemoryBSubgroup[indexInVectorizedTile + yInTile] = uint4(0, 0, 0, 0);
                    continue;
                }

                // It's fine that we only use the aligned version of readUint4() here, because the inputVector is the internal data which we can always
                // create it as an aligned array.
                uint4 value;

                // Note we can only use half type for matrix A and B, the value must be packed by 8 half type elements.
                accessUint4Aligned<AccessOp.READ, half, U, InputArray>(inoutVector, startIndex, value);
                sharedMemoryBSubgroup[indexInVectorizedTile + yInTile] = value;
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
                uint4 value = sharedMemoryBSubgroup[indexInVectorizedTile + yInTile];
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
            SPtr<uint4> paddingShmPtr = sharedMemoryBSubgroup + N * HeightInVectorTileB;

            uint index = laneId;
            for (uint i = 0; i < numLoadsEachThreadMatB; i++)
            {
                const uint xIndex = index / HeightInVectorTileB;
                if (xIndex >= numPaddingColumns)
                    break;
                paddingShmPtr[index] = uint4(0, 0, 0, 0);
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
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        InputArray inputVector)
            where U : __BuiltinFloatingPointType
            where InputArray : IArrayAccessor<U>
    {
        vectorLoadStore<U, InputArray, AccessOp.READ>(sharedMemoryB, tileIndex, subgroupIndex, inputVector);
    }

    // Load the input vector into shared memory. Each input vector is a column of the matrix B.
    // Since the input vector is loaded from thread local memory, one thread can only load one
    // column of the matrix B.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeVectorFromShB<U, InputArray, AccessOp Op = AccessOp.WRITE>(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        inout InputArray inoutVector)
            where U : __BuiltinFloatingPointType
            where InputArray : IArrayAccessor<U>
    {
        vectorLoadStore<U, InputArray, Op>(sharedMemoryB, tileIndex, subgroupIndex, inoutVector);
    }

    // Read one complete vector into the shared memory. Different from the loadVectorToShB,
    // this function is used to load the input vector into the shared memory for the transpose case, where each
    // tile will not contain the all the vectors in a warp. Instead, only partial warp will load the complete vector
    // into the shared memory. Compared to the LoadSharedMemoryFromLocalVector, where it loads partial vectors by every thread
    // in a warp.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadVectorForOuterProduct<int BatchSize, int ArraySizeInVector, int TileStrideInVector, U, InputArray>(
        SPtr<uint4> sharedMemory,
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
            else
            {
                accessUint4Aligned<AccessOp.READ, half, U>(inputVector, i * CMShape.ElementCountPerVector, value);
            }
            sharedMemory[index + i] = value;
        }
    }

    [require(cooperative_matrix, subgroup_basic)]
    internal static void sumReduceTiles<int ROW, int COLUMN>(inout MatC matC[ROW*COLUMN], in int subgroupId, SPtr<uint4> sharedMemory)
    {
        const int subgroupCount = getWaveCount();

        // perform n/2 sum reduces, so each iteration will perform a 2-way reduce operation,
        // totally log2(n) steps, where n is the number of subgroups.
        for (int k = 1; k < subgroupCount; k <<= 1)
        {
            // (subgroupId / k<< 1 ) calculate which cooperative matrix tile this warp will write to.
            // The whole shared memory is divided into subgroupCount/2 parts. So each two neighboring subgroups
            // will use one shared memory. And left subgroup loads, right subgroup stores.
            uint subgroupOffset = (subgroupId / (k<<1)) * (CMShape.CoopMatCSizeInVector * COLUMN);
            let ptrPerWarp = sharedMemory + subgroupOffset;
            [ForceUnroll]
            for (int i = 0; i < ROW; i++)
            {
                // Store one row of cooperative matrix to the shared memory.
                [ForceUnroll]
                for (int j = 0; j < COLUMN; j++)
                {
                    // This is the right node in the 2-way merge operation.
                    if (subgroupId % (k<<1) == k)
                    {
                        // We choose to process one row of cooperative matrices at a time, store the cooperative matrix in column major is more efficient.
                        // Because in column major, each column of the cooperative matrix is contiguous in the shared memory, so there will not be bank conflict.
                        matC[i * COLUMN + j].Store<RowMajor, uint4>(ptrPerWarp + j * CMShape.CoopMatCSizeInVector,
                                                CMShape.ROW_C / CMShape.ElementCountPerVectorMatC);
                    }
                }

                // wait for all subgroups finishing store the cooperative matrix to the shared memory.
                GroupMemoryBarrierWithGroupSync();

                // This the left node in the 2-way merge operation.
                if (subgroupId % (k<<1) == 0)
                {
                    // If the left node is the last subgroup, it means that there is no right node for it, so we don't need to do any addition for this subgroup.
                    if (subgroupId != (subgroupCount - 1))
                    {
                        [ForceUnroll]
                        for (int j = 0; j < COLUMN; j++)
                        {
                            MatC rightMatC = MatC.Load<RowMajor, uint4>(ptrPerWarp + j * CMShape.CoopMatCSizeInVector,
                                                CMShape.ROW_C / CMShape.ElementCountPerVectorMatC);

                            matC[i * COLUMN + j] = matC[i * COLUMN + j] + rightMatC;
                        }
                    }
                }
                // wait for all subgroups finishing the addition.
                GroupMemoryBarrierWithGroupSync();
            }
        }
    }

    // =========================================================================
    // Shared memory pack/unpack helpers
    //
    // Shared memory is typed as SPtr<uint4> (16 bytes per slot).
    // We cannot reinterpret-cast shared memory pointers in SPIR-V's logical
    // addressing model, so we manually pack/unpack scalar values of type U
    // into/from the uint4 array using bit_cast and shift operations.
    //
    // Layout: element index `i` of type U is stored at:
    //   uint4 slot:  i / ElementsPerUint4
    //   uint  comp:  (i % ElementsPerUint4) / ElementsPerUint
    //   sub-element: i % ElementsPerUint  (shift = subIdx * sizeof(U) * 8)
    // =========================================================================

    /// Store a scalar value of type U into uint4 shared memory at flat element index.
    [ForceInline]
    static void smemStore<U>(SPtr<uint4> mem, int index, U value)
        where U : __BuiltinFloatingPointType
    {
        static const int elementsPerUint  = 4 / sizeof(U);
        static const int elementsPerUint4 = 16 / sizeof(U);
        static const int bitsPerElement   = sizeof(U) * 8;

        const int slot = index / elementsPerUint4;
        const int comp = (index % elementsPerUint4) / elementsPerUint;
        const int sub  = index % elementsPerUint;
        const uint shift = sub * bitsPerElement;

        switch (sizeof(U))
        {
        case 2:
            {
                uint packed = mem[slot][comp];
                // Clear the target bits and insert new value
                packed = (packed & ~(uint(0xFFFF) << shift)) | (uint(bit_cast<uint16_t>(value)) << shift);
                mem[slot][comp] = packed;
            }
            break;
        case 4:
            mem[slot][comp] = bit_cast<uint>(value);
            break;
        }
    }

    /// Load a scalar value of type U from uint4 shared memory at flat element index.
    [ForceInline]
    static U smemLoad<U>(SPtr<uint4> mem, int index)
        where U : __BuiltinFloatingPointType
    {
        static const int elementsPerUint  = 4 / sizeof(U);
        static const int elementsPerUint4 = 16 / sizeof(U);
        static const int bitsPerElement   = sizeof(U) * 8;

        const int slot = index / elementsPerUint4;
        const int comp = (index % elementsPerUint4) / elementsPerUint;
        const int sub  = index % elementsPerUint;
        const uint shift = sub * bitsPerElement;

        switch (sizeof(U))
        {
        case 2:
            return bit_cast<U>(uint16_t((mem[slot][comp] >> shift) & 0xFFFF));
        case 4:
            return bit_cast<U>(mem[slot][comp]);
        default:
            return U(0);
        }
    }

    /// Load a packed uint from uint4 shared memory at a uint-level index.
    /// This is useful for reading two half values packed in a single uint.
    [ForceInline]
    static uint smemLoadUint(SPtr<uint4> mem, int uintIndex)
    {
        const int slot = uintIndex / 4;
        const int comp = uintIndex % 4;
        return mem[slot][comp];
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
        SPtr<uint4> sharedMemory,
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

        if (U is half)
        {
            // Half path: pack two half values into uint for efficient shared memory access
            // Uses vector<half, 2> atomic which maps to red.relaxed.gpu.global.add.noftz.f16x2 on CUDA

            for (int j = 0; j < NumIters; j++)
            {
                if (packed_col_idx < ColsPacked)
                {
                    // Sum packed half2 across all warps
                    half sum0 = half(0);
                    half sum1 = half(0);

                    for (int i = 0; i < subgroupCount; i++)
                    {
                        // Each warp's data is at offset i * ColsPacked (in half2/uint units)
                        uint packed = smemLoadUint(sharedMemory, packed_col_idx + i * ColsPacked);
                        half elem0 = bit_cast<half>(uint16_t(packed & 0xFFFF));
                        half elem1 = bit_cast<half>(uint16_t((packed >> 16) & 0xFFFF));
                        sum0 = sum0 + elem0;
                        sum1 = sum1 + elem1;
                    }

                    // Only write valid elements
                    int elemIdx0 = packed_col_idx * 2;
                    int elemIdx1 = elemIdx0 + 1;

                    if (elemIdx1 < M)
                    {
                        // Both elements are valid - use vector<half, 2> atomic
                        // TODO: vector<U,2> atomicAdd needs to be added to IPointerLikeAddress
                        vector<U, 2> sum_v2 = vector<U, 2>(U(sum0), U(sum1));
                        let addr = biasAddress.getOffset(elemIdx0);
                        addr.atomicAdd(0u, sum_v2);
                    }
                    else if (elemIdx0 < M)
                    {
                        // Only first element is valid (M is odd)
                        biasAddress.atomicAdd(uint(elemIdx0), U(sum0));
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
                        sum0 = sum0 + smemLoad<U>(sharedMemory, i * MPadded + elemIdx0);
                        if (elemIdx1 < MPadded)
                            sum1 = sum1 + smemLoad<U>(sharedMemory, i * MPadded + elemIdx1);
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

    // Sum reduce of the dOut across the block and accumulate to bias gradient.
    //
    // Uses __atomic_reduce_add which maps to PTX `red` instruction on CUDA:
    //   red.relaxed.gpu.global.add.noftz.f16x2 for half2
    // This is faster than standard atomicAdd because:
    // 1. Uses relaxed memory ordering (less synchronization overhead)
    // 2. Single instruction for two half values
    [ForceInline]
    [require(cuda_spirv, subgroup_partitioned)]
    [require(cuda_spirv, sm_6_6)]
    VISIBILITY_LEVEL static void sumReduceRows<U, Address, InArrayType>(
        SPtr<uint4> sharedMemory,
        in InArrayType dOut,
        in int subgroupId,
        Address biasAddress)
        where U : __BuiltinFloatingPointType
        where U.Differential == U
        where Address : IPointerLikeAddress<U>
        where InArrayType : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();
        const int subgroupCount = getWaveCount();

        // Phase 1: Warp-level reduction
        InArrayType localResult;
        [ForceUnroll]
        for (int i = 0; i < M; i++)
        {
            localResult[i] = WaveActiveSum(dOut[i]);
        }

        // Pad M to even number for half2 alignment
        static const int MPadded = (M + 1) & ~1;  // Round up to even
        static const int ColsPacked = MPadded / 2;  // Number of half2 pairs per warp
        static const int HasOddElement = M % 2;  // 1 if M is odd, 0 otherwise

        // Layout: [warp0: MPadded elements][warp1: MPadded elements]...
        // Shared memory stays as uint4; pack/unpack via smemStore/smemLoad helpers.

        // Phase 2: Lane 0 of each warp writes reduced result to shared memory
        if (laneId == 0)
        {
            [ForceUnroll]
            for (int i = 0; i < M; i++)
            {
                smemStore<U>(sharedMemory, subgroupId * MPadded + i, localResult[i]);
            }
            // Pad with zero if M is odd
            if (HasOddElement != 0)
            {
                smemStore<U>(sharedMemory, subgroupId * MPadded + M, U(0));
            }
        }

        GroupMemoryBarrierWithGroupSync();

        // Phase 3: Cross-warp reduction and atomic write
        crossWarpReduceAndAtomicWrite<U, Address, MPadded, ColsPacked>(
            sharedMemory, subgroupId, subgroupCount, biasAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    [require(spvAtomicFloat16AddEXT)]
    VISIBILITY_LEVEL static void outerProductAccumulate<U, Address, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
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

        // Batch size is either CoopMatA row count or CoopMatB column count, they are the same.
        static const int BatchSize = CMShape.COLUMN_A;
        static const int CoopMatCRows = (M + CMShape.ROW_A - 1) / CMShape.ROW_A;
        static const int CoopMatCColumns = (K + CMShape.COLUMN_A - 1) / CMShape.COLUMN_A;
        static const int StrideInVectorTileB = CoopMatCColumns * CMShape.COLUMN_B / CMShape.ElementCountPerVector;
        static const int StrideInVectorTileA = CoopMatCRows * CMShape.ROW_A / CMShape.ElementCountPerVector;
        static const int StrideInVectorTileC = CoopMatCColumns * CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC;

        MatC matC[CoopMatCRows * CoopMatCColumns];
        [ForceUnroll]
        for (int i = 0; i < CoopMatCRows * CoopMatCColumns; i++)
        {
            matC[i].fill(T(0.0f));
        }

        SPtr<uint4> warpLocalPtrA = sharedMemoryA + subgroupIndex * StrideInVectorTileA * BatchSize;
        SPtr<uint4> warpLocalPtrB = sharedMemoryB + subgroupIndex * StrideInVectorTileB * BatchSize;

        // Perform outer product for each warp.
        // The shared dimension is the Subgroup size N.
        [ForceUnroll]
        for (uint k = 0; k < N; k += CMShape.COLUMN_A)
        {
            uint tileIndex = k / CMShape.COLUMN_A;
            loadVectorForOuterProduct<BatchSize, TileInfo.Uint4AlignedM / CMShape.ElementCountPerVector, StrideInVectorTileA, TypeA, InArrayTypeA>(warpLocalPtrA, tileIndex, inputVectorA);
            loadVectorForOuterProduct<BatchSize, TileInfo.Uint4AlignedK / CMShape.ElementCountPerVector, StrideInVectorTileB, TypeB, InArrayTypeB>(warpLocalPtrB, tileIndex, inputVectorB);
            GroupMemoryBarrierWithWaveSync();

            MatA matA[CoopMatCRows];
            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                matA[i] = MatA.Load<ColumnMajor, uint4>( warpLocalPtrA + i * CMShape.ROW_A / CMShape.ElementCountPerVector, StrideInVectorTileA);
            }

            [ForceUnroll]
            for (uint j = 0; j < CoopMatCColumns; j++)
            {
                // For matrix B, stride will be the width of the tile. This is different from the forward pass in A*B where the stride is height of tile B.
                //  Since width of the tile B in outer product is K (input vector length), so the stride will be alignedK / ElementCountPerVector.
                let matB = MatB.Load<RowMajor, uint4>(warpLocalPtrB + j * CMShape.COLUMN_B / CMShape.ElementCountPerVector, StrideInVectorTileB);
                for (uint i = 0; i < CoopMatCRows; i++)
                {
                    int index = i * CoopMatCColumns + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, CMShape.ROW_A, CMShape.COLUMN_A, CMShape.COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        // sum reduce will accumulate the result cross all the warps into the cooperative matrix in the first subgroup.
        sumReduceTiles<CoopMatCRows, CoopMatCColumns>(matC, subgroupIndex, sharedMemoryB);

        // Only the first subgroup need to store the result to the global memory.

        if (subgroupIndex == 0)
        {
            // Store the result shared memory. We store one row of the cooperative matrix at a time.
            [ForceUnroll]
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                [ForceUnroll]
                for (uint j = 0; j < CoopMatCColumns; j++)
                {
                    // Save the result to shared memory in row major, because this is cache friendly for global memory access.
                    // As each row of the tile will be corresponding to the contiguous global memory.
                    matC[i * CoopMatCColumns + j].Store<RowMajor, uint4>(sharedMemoryB + j * CMShape.COLUMN_C / CMShape.ElementCountPerVectorMatC, (uint)StrideInVectorTileC);

                }

                // We will leverage the fact that the result the outerproduct is the same shape as matrix A. But, the tile organization is different.
                // In forward pass, the tile in the shared memory is in shape M x T_ROW_A, which is one column of cooperative matrices.
                // while the result here is in shape T_ROW_A x K, which is one row of cooperative matrices.
                // So we can use transposed version of LoadShA/StoreShA to store the result. Because in transpose version, the major-ness is along
                // K dimension. Note that column major in transpose case is just the row major in non-transpose case.

                typealias MMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, true>;
                MMA.storeShA<U, Address>(sharedMemoryB, i, weightAddress);

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
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        SPtr<uint4> sharedMemoryC,
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
        SPtr<uint4> ptrA[EnableDoubleBuffer ? 2 : 1];
        SPtr<uint4> ptrB[EnableDoubleBuffer ? 2 : 1];
        SPtr<uint4> ptrC = sharedMemoryC;
        const uint subgroupIndex = getWaveId();

        if (!EnableDoubleBuffer)
        {
            ptrA[0] = sharedMemoryA;
            ptrB[0] = sharedMemoryB;
        }
        else
        {
            ptrA[0] = sharedMemoryA;
            ptrA[1] = sharedMemoryA + ShMemInfo.SharedMemSizeInVectorMatA;
            ptrB[0] = sharedMemoryB;
            ptrB[1] = sharedMemoryB + ShMemInfo.SharedMemSizeInVectorMatB;
            // PreLoad for the first iteration
            loadShA<U, Address>(ptrA[0], 0, weightAddress);
            loadVectorToShB(ptrB[0], 0, subgroupIndex, inputVector);
        }

        // fetch first tile of Matrix A and Matrix B into shared memory
        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        uint bufferIndex = 0;
        MatC matC[TileInfo.NCoopMat];
        [ForceUnroll]
        for (int i = 0; i < TileInfo.NCoopMat; i++)
        {
            matC[i].fill(T(0.0f));
        }

        OutArrayType outputVector;
        if (Bias)
        {
            [ForceUnroll]
            for (int i = 0; i < M; i++)
            {
                U bias = biasAddress.value[uint(i)];
                outputVector[i] = __realCast<V>(bias);
            }
        }

        [ForceUnroll]
        for (int k = 0; k < TileInfo.SharedDimensionSize; k += CMShape.COLUMN_A)
        {
            uint tileIndex = k / CMShape.COLUMN_A;

            if (EnableDoubleBuffer)
            {
                GroupMemoryBarrierWithGroupSync();

                // swap to next buffer to load new data.
                bufferIndex = bufferIndex ^ 1;
                // Load the another buffer and do the math so we can hide the latency of the load.
                loadShA<U, Address>(ptrA[bufferIndex], tileIndex, weightAddress);
                loadVectorToShB(ptrB[bufferIndex], tileIndex, subgroupIndex, inputVector);

                // swap back to the previous buffer to perform the math.
                bufferIndex = bufferIndex ^ 1;
            }
            else
            {
                loadShA<U, Address>(ptrA[0], tileIndex, weightAddress);
                loadVectorToShB(ptrB[0], tileIndex, subgroupIndex, inputVector);

                // The reason that we have to sync the whole workgroup is that for matrix A, the tile is shared by
                // whole workgroup.

                // TODO:
                // The alternative solution to duplicate the tile A to each subgroup, so we will only need warp sync here.
                // But that will waste lots of shared memory and increase memory transactions. We need benchmark to see
                // which solution is better.
                GroupMemoryBarrierWithGroupSync();
            }

            // Math loop: This operator is executed by each warp (sub-group). shA is shared by all threads in the workgroup, while
            // shB is shared only by each subgroup, and each subgroup will have its own offset on shB.
            // For matB, each warp could only have 1 or 2 Tiles to load according whether this warp is less (1 tile) or more than half warp (2 tiles).
            MatA matA[TileInfo.NCoopMatRow];
            [ForceUnroll]
            for (uint i = 0; i < TileInfo.NCoopMatRow; i ++)
            {
                if (TransposeA)
                {
                    // Really be careful the stride of tile A in the transpose case. The tile is column major, and the cooperative matrix is stacked in columns of the tile.
                    // So the stride is actually the height the tile. The offset between two cooperative matrices is just height of the cooperative matrix.
                    matA[i] = MatA.Load<ColumnMajor, uint4>(ptrA[bufferIndex] + i * CMShape.ROW_A/CMShape.ElementCountPerVector, TileInfo.HeightInVectorTileA);
                }
                else
                {
                    matA[i] = MatA.Load<RowMajor, uint4>(ptrA[bufferIndex] + i * CMShape.CoopMatASizeInVector, TileInfo.WidthInVectorTileA);
                }
            }

            [ForceUnroll]
            for (uint j = 0; j < TileInfo.NCoopMatColumn; j ++)
            {
                // NTilesColumn * HeightInVectorTileB is the size of the shared memory for matrix B in one warp measured in vector uint4.
                SPtr<uint4> ptrPerWarp = ptrB[bufferIndex] + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatB;
                let matB = MatB.Load<ColumnMajor, uint4>(ptrPerWarp + j * CMShape.CoopMatBSizeInVector, TileInfo.HeightInVectorTileB);

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

        // Get start address of the shared memory for current warp.
        SPtr<uint4> ptrPerWarp = ptrC + subgroupIndex * ShMemInfo.SharedMemSizeInVectorMatC;

        [ForceUnroll]
        for (int i = 0; i < TileInfo.NCoopMatRow; i ++)
        {
            [ForceUnroll]
            for (int j = 0; j < TileInfo.NCoopMatColumn; j ++)
            {
                int index = i * TileInfo.NCoopMatColumn + j;
                matC[index].Store<ColumnMajor, uint4>(ptrPerWarp + j * CMShape.CoopMatCSizeInVector,
                                TileInfo.HeightInVectorTileC);
            }

            if (Bias)
            {
                storeVectorFromShB<V, OutArrayType, AccessOp.ACCUMULATE>(ptrC, i, subgroupIndex, outputVector);
            }
            else
            {
                storeVectorFromShB<V, OutArrayType, AccessOp.WRITE>(ptrC, i, subgroupIndex, outputVector);
            }

            // wati until all threads in the warp finish reading shared memory and storing to local vector.
            GroupMemoryBarrierWithWaveSync();
        }

        return outputVector;
    }
}

// Cooperative matrix is only supported by CUDA and SPIR-V
// WaveTangledVector is a vector type that emulates the Cooperative Vector type by using Cooperative Matrix feature which is
// supported by CUDA and SPIR-V.
[require(cooperative_matrix, subgroup_basic)]
public struct WaveTangledVector<T, ShMemPool : ISharedMemoryPool, int N, int SubgroupSize> : IVector<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = WaveTangledVector<T.Differential, ShMemPool, N, SubgroupSize>;

    public static const int Size = N;
    public no_diff ShMemPool shMemPool;

    private typealias DTypeMatC = half;

    // TODO: This is just an easiest solution to make the alignment work. But the disadvantage is wasting of registers.
    //       A better solution is that we can implement a "special" alignment rule for the vectorized reader.
    //       Normally, the alignment rule is that one uint4 can pack 8 half type elements, so we want our internal data
    //       is always 8 elements aligned. But we can have a "byte-aligned" rule such that we just need our data is 16 bytes aligned.
    //       So for example, float[4] is also considered as aligned. We can just pack 4 elements into a uint4, the remaining 4 elements
    //       just fill with 0.
    // Since we always use half type for A and B, we want to follow the alignment requirement of half type.
    internal static const uint ElementCountPerVector = sizeof(uint4) / sizeof(DTypeMatC);
    internal static const int Uint4AlignedInputSize = ((N + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    // [DerivativeMember(Differential.data)]
    internal T[Uint4AlignedInputSize] data;
    public int getCount() { return N; }

    public __init() { data = {}; }

    public __init(T value)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            this.data[i] = value;
        }

        cleanPaddingData();
    }

    public __init(T[Size] inData)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            this.data[i] = inData[i];
        }

        cleanPaddingData();
    }

    internal __init(T[Uint4AlignedInputSize] inData)
    {
        this.data = inData;
    }

    public __init(This other) { this.data = other.data; }

    public __init<InputArray : IArray<T>>(InputArray inData)
    {
        [ForceUnroll]
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

        [ForceUnroll]
        for (int i = N; i < alignedSize; i++)
        {
            this.data[i] = T(0.0f);
        }
    }

    public __subscript(int index) -> T
    {
        [ForceInline]
        get { return this.data[index]; }

        [ForceInline]
        set { this.data[index] = newValue; }
    }

    [ForceInline]
    private OutputVector linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum Target, bool Bias>(
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T.Differential>
        where OutputVector : IVector<T>
    {
        typealias MMA = MMAHelper<DTypeMatC, N, OutputVector.Size, SubgroupSize, Target, false>;
        SPtr<uint4> shA = shMemPool.getPointer();
        SPtr<uint4> shB = shA + MMA.ShMemInfo.SharedMemSizeInVectorMatA;
        SPtr<uint4> shC = shB;
        const int AlignedOutSize = MMA.TileInfo.Uint4AlignedM;
        let outputArray = MMA.mma<T, Address, T[Uint4AlignedInputSize], T[AlignedOutSize], T, Bias
                            >( data, shA, shB, shC, weightAddress, biasAddress);
        return OutputVector(outputArray);
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
    {
        typealias MMA = MMAHelper<DTypeMatC, N, OutputVector.Size, SubgroupSize, Target, true>;

        SPtr<uint4> shA = dthis.p.shMemPool.getPointer();
        SPtr<uint4> shB = shA + MMA.ShMemInfo.SharedMemSizeInVectorMatA;
        SPtr<uint4> shC = shB;

        // In backward, output size is K dimension.
        const int AlignedOutSize = MMA.TileInfo.Uint4AlignedK;
        const int AlignedInputSize = MMA.TileInfo.Uint4AlignedM;

        // This.Differential is the derivative of the input vector, which is the output
        // of the mma operation.
        // dIn = W^T * dOut;
#if 1
        let outArray = MMA.mma<T, Address, OutputVector.Differential, T[AlignedOutSize].Differential, T.Differential, false
                        >( doutput, shA, shB, shC, dWeightAddress.p, none);
        This.Differential dInput = This.Differential(outArray);
        dthis = DifferentialPair<This>(dthis.p, dInput);

        // dW = dOut * input^T
        MMA.outerProductAccumulate<T.Differential, Address.Differential,
                                   T.Differential, OutputVector.Differential,
                                   T, T[Uint4AlignedInputSize]
                                  >( shA, shB, doutput, dthis.p.data, dWeightAddress.d);
#else
        let dInput = MMA.mma<T, Address, OutputVector.Differential, This.Differential
                        >( doutput, shA, shB, shC, dWeightAddress.p, none);
        dthis = DifferentialPair<This>(dthis.p, dInput);

        MMA.outerProductAccumulate<T.Differential, Address.Differential,
                                    T.Differential, OutputVector.Differential,
                                    T, This
                                  >( shA, shB, doutput, dthis.p, dWeightAddress.d);
#endif

        if (Bias)
        {
            const int subgroupIndex = getWaveId();
            MMA.sumReduceRows<T.Differential, Address.Differential, OutputVector.Differential
                >(shA, doutput, subgroupIndex, biasAddress.value.d);
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
