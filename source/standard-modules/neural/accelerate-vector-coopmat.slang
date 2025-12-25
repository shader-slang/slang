// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#ifndef UNIT_TEST
#define VISIBILITY_LEVEL public
#else
#define VISIBILITY_LEVEL internal
#endif

VISIBILITY_LEVEL enum TargetEnum: uint32_t
{
    CUDA = 0,
    SPIR_V = 1,
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
internal int getWaveCount()
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

// A wrapper of matrix type to access each tile of the matrix
// Note: The reason we want to make SubgroupSize as a generic parameter instead of using WaveGetLaneCount() is that
//       we need the logic to be expand as much as possible during compile time. WaveGetLaneCount() is a runtime function,
//       so if we use it directly in our mma function, some of branches won't be eliminated during compile time.
[require(cooperative_matrix)]
VISIBILITY_LEVEL struct MMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, bool TransposeA = false>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int T_ROW_A = 16;
    static const int T_COLUMN_A = 16;

    // TODO: Currently, we only support floating point data type, therefore, we can always use 16x16x16.
    //  Once in the future we want to extend to support integer type, we can uncomment the following code.
    static const int T_ROW_B = 16; // (Target == TargetEnum.CUDA) ? 16 : (sizeof(T) == 1 ? 32 : 16);
    static const int T_COLUMN_B = 16;

    static const int T_ROW_C = 16;
    static const int T_COLUMN_C = 16;

    static const int T_SIZE_A = T_ROW_A * T_COLUMN_A;
    static const int T_SIZE_B = T_ROW_B * T_COLUMN_B;
    static const int T_SIZE_C = T_ROW_C * T_COLUMN_C;

    static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;
    static const linalg.CoopMatMatrixLayout ColumnMajor = linalg.CoopMatMatrixLayout.ColumnMajor;

    // Weight matrix is in shape of M x K
    // Input vector will be collected by each thread in a sub-group to form a matrix in shape of K x N
    // Note, M, K, N are measured in T.
    // Since Cooperative matrix is only worked for a whole subgroup (warp), so we always use the SubgroupSize as the N value.
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    // Because the vectorized load is along K dimension (row of A and column of B), so we need to check
    // if K is aligned with the vector size (ElementCountPerVector), we can use the vectorized load,
    // otherwise, we need to use the scalar load.
    // ElementCountPerVector is measured in T.
    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);

    // Since A and B can only be half type, ElementCountPerVector is fixed. However, C can be both half and float.
    static const uint ElementCountPerVectorMatC = sizeof(uint4) / sizeof(T);

    static const bool IsAlignedVector = (K % ElementCountPerVector) == 0;

    // For the Tile A, how many cooperative matrices in a row
    static const int NCoopMatRow = TransposeA ? (K + T_COLUMN_A - 1) / T_COLUMN_A :
                                                (M + T_ROW_A - 1) / T_ROW_A;


    // Notice that because of our specific workload, NTilesColumn is always warp-size (as long as it's more than half warp) or half warp (<= half warp).
    // For the Tile B, how many cooperative matrices in a column
    static const int NCoopMatColumn = (N + T_COLUMN_B - 1) / T_COLUMN_B;

    // Total number of cooperative matrices that consist the result of C = A * B
    static const int NCoopMat = NCoopMatRow * NCoopMatColumn;

    // When A * B, k is the shared dimension, and when A^T * B, m is the shared dimension.
    static const uint SharedDimensionSize = TransposeA ? Uint4AlignedM : Uint4AlignedK;

    // `Uint4AlignedK` is the K value that is aligned with the uint4 vector size. Because we always do the vectorized load
    // along K dimension, so we need to align the K value with the vector size.
    // Measured in T.
    VISIBILITY_LEVEL static const int Uint4AlignedK = ((K + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;
    VISIBILITY_LEVEL static const int Uint4AlignedM = ((M + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    // The height of tile A measure in T and uint4.
    static const uint HeightInElementsTileA = NCoopMatRow * T_ROW_A;
    static const uint HeightInVectorTileA = HeightInElementsTileA / ElementCountPerVector;

    // In order to achieve the high throughput, we will write data to shared memory in vectorized way.
    // e.g. if T is half, it's 2 bytes, so uint4 can store 8 elements.
    // Since the size in bytes of the tile in one row is T.COLUMN * sizeof(T),
    // the number of vectorized columns in one row of cooperative matrix A is T.COLUMN * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4.
    static const uint WidthInVectorTileA = T_COLUMN_A / ElementCountPerVector;

    // WidthInElementTileA is the number of elements in one row of the tile in original data type.
    static const uint WidthInElementTileA = T_COLUMN_A;

    // Similarly, the number of vectorized rows in one column of cooperative matrix B is T.ROW * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    static const uint HeightInVectorTileB = T_ROW_B / ElementCountPerVector;

    // So HeightInElementTileB is the number of elements in one column of the tile, measured in T.
    static const uint HeightInElementTileB = HeightInVectorTileB * ElementCountPerVector;

    // The height of tile B measure in T and uint4.
    static const int WidthInElementsTileB = NCoopMatColumn * T_COLUMN_B;
    static const int WidthInVectorTileB = WidthInElementsTileB / ElementCountPerVector;

    // The aligned size of the shared memory for matrix A and B.
    // The size of the shared memory is the tile size for each matrices, measured in uint4.
    // Tile A = AlignedM * Tile_width of A
    // Tile B = Tile_height of B * AlignedN
    // Tile C = Tile_height of B * AlignedN
    // Note B and C are the same size because their widths are always the same as subGroupSize and
    // their heights are fixed determined by Cooperative matrix shape.
    VISIBILITY_LEVEL static const uint SharedMemSizeInVectorMatA = (HeightInElementsTileA * T_COLUMN_A) / ElementCountPerVector;
    VISIBILITY_LEVEL static const uint SharedMemSizeInVectorMatB = (T_ROW_B * WidthInElementsTileB) / ElementCountPerVector;

    // In the actual implementation, we always reuse shared memory B for store MatC. However, if data type of MatC is float,
    // we will need double size of shared memory B for C.
    static const uint SharedMemSizeInVectorMatC = SharedMemSizeInVectorMatB * sizeof(T) / sizeof(half);

    static const uint CoopMatASizeInVector = T_SIZE_A / ElementCountPerVector;
    static const uint CoopMatBSizeInVector = T_SIZE_B / ElementCountPerVector;
    static const uint CoopMatCSizeInVector = T_SIZE_C / ElementCountPerVector;

    // NumLoadsEachThreadMatA is the number of loads each thread needs to perform.
    // N is the number of active threads in the subgroup.
    // For matrix A, each iteration we load a column of matrix tile of Matrix A, which is M x 32 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to
    // (M * 32) / (N * 16) = M * 2 / N, and we just round up the result to the nearest integer.
    static const uint NumLoadsEachThreadMatA = (HeightInElementsTileA * 2 + N - 1) / N;

    // Similarly, for matrix A^T, each iteration we load a column of matrix tile of Matrix A^T, which is K x 32 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to
    // (K * 32) / (N * 16) = K * 2 / N, and we just round up the result to the nearest integer.
    static const uint NumLoadsEachThreadMatATranspose = (HeightInElementsTileA * 2 + N - 1) / N;

    // OffsetPerThreadLoadMatA is the offset between each load.
    // The reason is same as above, since the whole subgroup can load N * 16 bytes, and the load is consecutive,
    // so the offset is the N * 16 bytes, and we measure it in uint4 units, so it's N.
    static const uint OffsetPerThreadLoadMatA = N;

    // A/B can only be half, so we don't need to use generic parameter here.
    typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, T_ROW_A,    T_COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, T_COLUMN_B, T_COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T,    MemoryScope.Subgroup, T_ROW_C,    T_COLUMN_C, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    // TODO: This will always double the buffer usage, therefore we haven't really implemented the double buffer yet.
    static const bool EnableDoubleBuffer = false;

    [ForceInline]
    static uint flattenedIndex(uint2 coord, int stride)
    {
        if (TransposeA)
        {
            return coord.y + coord.x * stride;
        }
        else
        {
            return coord.x + coord.y * stride;
        }
    }

    [ForceInline]
    static uint2 vectorizedTileCoordToWeightCoord(uint2 coordInTile, int tileIndex)
    {
        uint2 coordOut;
        if (TransposeA)
        {
            coordOut.x = coordInTile.x + tileIndex * WidthInElementTileA;
            coordOut.y = coordInTile.y * ElementCountPerVector;
        }
        else
        {
            coordOut.x = coordInTile.x * ElementCountPerVector + tileIndex * WidthInElementTileA;
            coordOut.y = coordInTile.y;
        }
        return coordOut;
    }

    [ForceInline]
    static uint2 vectorizedTileIndexTo2DCoord(uint indexInVectorizedTile)
    {
        uint2 coordOut;
        if (TransposeA)
        {
            coordOut.x = indexInVectorizedTile / HeightInVectorTileA;
            coordOut.y = indexInVectorizedTile % HeightInVectorTileA;
        }
        else
        {
            coordOut.x = indexInVectorizedTile % WidthInVectorTileA;
            coordOut.y = indexInVectorizedTile / WidthInVectorTileA;
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
    VISIBILITY_LEVEL static void MatALoadStore<U, Storage, bool isStore = false>(
            SPtr<uint4> sharedMemory,
            int tileIndex,
            Storage weightStorage,
            Storage.Address weightAddress)
                where U : __BuiltinFloatingPointType
                where U.Differential == U
                where Storage : IStorage<U>
    {
        uint indexInVectorizedTile = WaveGetLaneIndex();

        // get the 2-D coordinate of the tile in the vectorized tile
        const uint2 coordInTile = vectorizedTileIndexTo2DCoord(indexInVectorizedTile);

        // tx and ty are the 2-D coordinate of scalar tile.
        // row coordinate ty is the same between vectorized tile and original tile.
        // column coordinate tx needs to be scaled by ElementCountPerVector to get the actual column coordinate in original data type.
        uint2 coordInWeight = vectorizedTileCoordToWeightCoord(coordInTile, tileIndex);

        uint4 value;
        uint indexInWeight;
        for (uint i = 0; i < NumLoadsEachThreadMatA; i++)
        {
            if (coordInWeight.y >= HeightInElementsTileA)
                break;
            // Given the coordinate inside the tile and the tile index, we can get the index in weight matrix.
            // Note, we always treat row of matrix A as uint4 aligned, so always calculate 'indexInWeight' as if it's uint4 aligned.
            // though it might not. But IStorage.readUint4() will handle the padding.
            // We only need to provide the actual boundary and the aligned boundary.
            indexInWeight = flattenedIndex(coordInWeight, K);

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
                    let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
                    // Though the weight matrix is not necessarily aligned with the uint4 vector size, readUnit4() will handle the padding
                    // if the reading is cross the boundary of the uint4 vector size.
                    value = weightStorage.readUint4<T, IsAlignedVector, K>(weightAddress, offsetAddress);
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
                    let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
                    weightStorage.writeUint4Atomic<T, IsAlignedVector, K>(weightAddress, offsetAddress, value);
                }
            }

            indexInVectorizedTile += OffsetPerThreadLoadMatA;

            // Given the index in the tile, get the 2-D coordinate in the matrix A.
            // Note that, when TransposeA is true, this is 2-D coordinate in the matrix A^T. We don't change the semantics of the 2-D coordinate
            // in matrix, but we change the way to calculate the 1-D index in the matrix. But we can still use the same function to get the 2-D coordinate.
            coordInWeight = vectorizedTileCoordToWeightCoord(vectorizedTileIndexTo2DCoord(indexInVectorizedTile), tileIndex);
        }
    }

    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadShA<U, Storage>(
        SPtr<uint4> sharedMemory,
        int tileIndex,
        Storage weightStorage,
        Storage.Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Storage : IStorage<U>
    {
        MatALoadStore<U, Storage, false>(sharedMemory, tileIndex, weightStorage, weightAddress);
    }

    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeShA<U, Storage>(
        SPtr<uint4> sharedMemory,
        int tileIndex,
        Storage weightStorage,
        Storage.Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Storage : IStorage<U>
    {
        MatALoadStore<U, Storage, true>(sharedMemory, tileIndex, weightStorage, weightAddress);
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void vectorLoadStore<U, InputArray, bool isStore = false>(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        inout InputArray inoutVector)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where InputArray : IArrayAccessor<U>
    {
        const uint laneId = WaveGetLaneIndex();
        SPtr<uint4> sharedMemoryBSubgroup = sharedMemoryB + subgroupIndex * SharedMemSizeInVectorMatB;

        // start index in vectorized tile for current thread: x * column_stride
        const uint indexInVectorizedTile = laneId * HeightInVectorTileB;
        const uint yOffset = (tileIndex * HeightInVectorTileB) * ElementCountPerVector;

        for (uint yInTile = 0; yInTile < HeightInVectorTileB; yInTile++)
        {
            const int startIndex = yInTile * ElementCountPerVector + yOffset;

            // Bounds check on Y direction. If y coordinate out of the input vector length, just padding with 0.
            // No need to check the X direction, because the X direction is bound by the thread count, so any active
            // thread will definitely have its thread-local vector available.

            bool isOutOfRange;
            if (isStore)
            {
                // for vector store, the length of output vector has length of M in normal case
                // and K in transpose case.
                isOutOfRange = TransposeA ? (startIndex >= K) : (startIndex >= M);

                // No padding needs for writing back to local vector.
                if (isOutOfRange)
                    return;

                accessUint4Aligned<AccessOp.WRITE, half, U>(inoutVector, startIndex, sharedMemoryBSubgroup[indexInVectorizedTile + yInTile]);
            }
            else
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
                accessUint4Aligned<AccessOp.READ, half, U>(inoutVector, startIndex, value);
                sharedMemoryBSubgroup[indexInVectorizedTile + yInTile] = value;
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
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void loadVectorToShB<InputArray>(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        InputArray inputVector)
            where InputArray : IArrayAccessor<T>
    {
        vectorLoadStore<T, InputArray, false>(sharedMemoryB, tileIndex, subgroupIndex, inputVector);
    }

        // Load the input vector into shared memory. Each input vector is a column of the matrix B.
    // Since the input vector is loaded from thread local memory, one thread can only load one
    // column of the matrix B.
    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void storeVectorFromShB<InputArray>(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        out InputArray outputVector)
            where InputArray : IArrayAccessor<T>
    {
        vectorLoadStore<T, InputArray, true>(sharedMemoryB, tileIndex, subgroupIndex, outputVector);
    }

    // Read one complete vector into the shared memory. Different from the loadVectorToShB,
    // this function is used to load the input vector into the shared memory for the transpose case, where each
    // tile will not contain the all the vectors in a warp. Instead, only partial warp will load the complete vector
    // into the shared memory. Compared to the LoadSharedMemoryFromLocalVector, where it loads partial vectors by every thread
    // in a warp.
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

        for (uint i = 0; i < TileStrideInVector; i++)
        {
            uint4 value;
            if (i >= ArraySizeInVector)
            {
                value = uint4(0, 0, 0, 0);
            }
            else
            {
                accessUint4Aligned<AccessOp.READ, half, U>(inputVector, i * ElementCountPerVector, value);
            }
            sharedMemory[index + i] = value;
        }
    }

    [require(cooperative_matrix, subgroup_basic)]
    internal static void sumReduce<int ROW, int COLUMN>(inout MatC matC[ROW*COLUMN], in int subgroupId, SPtr<uint4> sharedMemory)
    {
        const int subgroupCount = getWaveCount();

        // perform n/2 ways warp reduce, so each iteration will perform a 2-way reduce operation,
        // totally log2(n) steps, where n is the number of subgroups.
        for (int k = 1; k < subgroupCount; k <<= 1)
        {
            // (subgroupId / k<< 1 ) calculate which cooperative matrix tile this warp will write to.
            // The whole shared memory is divided into subgroupCount/2 parts. So each two neighboring subgroups
            // will use one shared memory. And left subgroup loads, right subgroup stores.
            uint subgroupOffset = (subgroupId / (k<<1)) * (CoopMatCSizeInVector * COLUMN);
            let ptrPerWarp = sharedMemory + subgroupOffset;
            for (int i = 0; i < ROW; i++)
            {
                // Store one row of cooperative matrix to the shared memory.
                for (int j = 0; j < COLUMN; j++)
                {
                    // This is the right node in the 2-way merge operation.
                    if (subgroupId % (k<<1) == k)
                    {
                        // We choose to process one row of cooperative matrices at a time, store the cooperative matrix in column major is more efficient.
                        // Because in column major, each column of the cooperative matrix is contiguous in the shared memory, so there will not be bank conflict.
                        matC[i * COLUMN + j].Store<RowMajor, uint4>(ptrPerWarp + j * CoopMatCSizeInVector, T_ROW_C / ElementCountPerVector);
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
                        for (int j = 0; j < COLUMN; j++)
                        {
                            MatC rightMatC = MatC.Load<RowMajor, uint4>(ptrPerWarp + j * CoopMatCSizeInVector, T_ROW_C / ElementCountPerVector);
                            matC[i * COLUMN + j] = matC[i * COLUMN + j] + rightMatC;
                        }
                    }
                }
                // wait for all subgroups finishing the addition.
                GroupMemoryBarrierWithGroupSync();
            }
        }
    }

    [require(cooperative_matrix, subgroup_basic)]
    [require(spvAtomicFloat16AddEXT)]
    VISIBILITY_LEVEL static void outerProductAccumulate<U, Storage, TypeA, InArrayTypeA, TypeB, InArrayTypeB>(
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        InArrayTypeA inputVectorA,
        InArrayTypeB inputVectorB,
        Storage weightStorage,
        Storage.Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Storage : IStorage<U>
            where TypeA : __BuiltinFloatingPointType
            where TypeA.Differential == TypeA
            where TypeB : __BuiltinFloatingPointType
            where TypeB.Differential == TypeB
            where InArrayTypeA : IArrayAccessor<TypeA>
            where InArrayTypeB : IArrayAccessor<TypeB>
    {
        const uint subgroupIndex = getWaveId();

        // Batch size is either CoopMatA row count or CoopMatB column count, they are the same.
        static const int BatchSize = T_COLUMN_A;
        static const int CoopMatCRows = (M + T_ROW_A - 1) / T_ROW_A;
        static const int CoopMatCColumns = (K + T_COLUMN_A - 1) / T_COLUMN_A;
        static const int StrideInVectorTileB = CoopMatCColumns * T_COLUMN_B / ElementCountPerVector;
        static const int StrideInVectorTileA = CoopMatCRows * T_ROW_A / ElementCountPerVector;

        MatC matC[CoopMatCRows * CoopMatCColumns];
        for (int i = 0; i < CoopMatCRows * CoopMatCColumns; i++)
        {
            matC[i].fill(T(0.0f));
        }

        SPtr<uint4> warpLocalPtrA = sharedMemoryA + subgroupIndex * StrideInVectorTileA * BatchSize;
        SPtr<uint4> warpLocalPtrB = sharedMemoryB + subgroupIndex * StrideInVectorTileB * BatchSize;

        // Perform outer product for each warp.
        // The shared dimension is the Subgroup size N.
        for (uint k = 0; k < N; k += T_COLUMN_A)
        {
            uint tileIndex = k / T_COLUMN_A;
            loadVectorForOuterProduct<BatchSize, Uint4AlignedM / ElementCountPerVector, StrideInVectorTileA, TypeA, InArrayTypeA>(warpLocalPtrA, tileIndex, inputVectorA);
            loadVectorForOuterProduct<BatchSize, Uint4AlignedK / ElementCountPerVector, StrideInVectorTileB, TypeB, InArrayTypeB>(warpLocalPtrB, tileIndex, inputVectorB);
            GroupMemoryBarrierWithWaveSync();

            MatA matA[CoopMatCRows];
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                matA[i] = MatA.Load<ColumnMajor, uint4>( warpLocalPtrA + i * T_ROW_A / ElementCountPerVector, StrideInVectorTileA);
            }

            for (uint j = 0; j < CoopMatCColumns; j++)
            {
                // For matrix B, stride will be the width of the tile. This is different from the forward pass in A*B where the stride is height of tile B.
                //  Since width of the tile B in outer product is K (input vector length), so the stride will be alignedK / ElementCountPerVector.
                let matB = MatB.Load<RowMajor, uint4>(warpLocalPtrB + j * T_COLUMN_B / ElementCountPerVector, StrideInVectorTileB);
                for (uint i = 0; i < CoopMatCRows; i++)
                {
                    int index = i * CoopMatCColumns + j;
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, T_ROW_A, T_COLUMN_A, T_COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        GroupMemoryBarrierWithGroupSync();
        // sum reduce will accumulate the result cross all the warps into the cooperative matrix in the first subgroup.
        sumReduce<CoopMatCRows, CoopMatCColumns>(matC, subgroupIndex, sharedMemoryB);

        if (subgroupIndex == 0)
        {
            // Store the result shared memory. We store one row of the cooperative matrix at a time.
            for (uint i = 0; i < CoopMatCRows; i++)
            {
                for (uint j = 0; j < CoopMatCColumns; j++)
                {
                    // Save the result to shared memory in row major, because this is cache friendly for global memory access.
                    // As each row of the tile will be corresponding to the contiguous global memory.
                    matC[i * CoopMatCColumns + j].Store<RowMajor, uint4>(sharedMemoryB + j * T_COLUMN_B / ElementCountPerVector, StrideInVectorTileB);
                }

                // We will leverage the fact that the result the outerproduct is the same shape as matrix A. But, the tile organization is different.
                // In forward pass, the tile in the shared memory is in shape M x T_ROW_A, which is one column of cooperative matrices.
                // while the result here is in shape T_ROW_A x K, which is one row of cooperative matrices.
                // So we can use transposed version of LoadShA/StoreShA to store the result. Because in transpose version, the major-ness is along
                // K dimension. Note that column major in transpose case is just the row major in non-transpose case.
                typealias MMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, true>;
                MMA.storeShA<U, Storage>(sharedMemoryB, i, weightStorage, weightAddress);
            }
        }
    }

    // The workload is different from the large size matrix-matrix multiply, we actually perform the matrix-vector multiply
    //  for each thread, so the matrix-matrix multiply only needs to be performed for each warp (sub-group).
        [require(cooperative_matrix, subgroup_basic)]
        VISIBILITY_LEVEL static OutArrayType mma<U, Storage, InArrayType, OutArrayType>(InArrayType inputVector,
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        SPtr<uint4> sharedMemoryC,
        Storage weightStorage,
        Storage.Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Storage : IStorage<U>
            where InArrayType : IArrayAccessor<T>
            where OutArrayType : IArrayAccessor<T>
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
            ptrA[1] = sharedMemoryA + SharedMemSizeInVectorMatA;
            ptrB[0] = sharedMemoryB;
            ptrB[1] = sharedMemoryB + SharedMemSizeInVectorMatB;
            // PreLoad for the first iteration
            loadShA<U, Storage>(ptrA[0], 0, weightStorage, weightAddress);
            loadVectorToShB(ptrB[0], 0, subgroupIndex, inputVector);
        }

        // fetch first tile of Matrix A and Matrix B into shared memory
        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        uint bufferIndex = 0;

        MatC matC[NCoopMat];

        for (int i = 0; i < NCoopMat; i++)
        {
            matC[i].fill(T(0.0f));
        }

        for (int k = 0; k < SharedDimensionSize; k += T_COLUMN_A)
        {
            uint tileIndex = k / T_COLUMN_A;

            if (EnableDoubleBuffer)
            {
                // warp sync to ensure all threads in the warp have loaded the data.
                // The reason we always put
                GroupMemoryBarrierWithWaveSync();

                // swap to next buffer to load new data.
                bufferIndex = bufferIndex ^ 1;
                // Load the another buffer and do the math so we can hide the latency of the load.
                loadShA<U, Storage>(ptrA[bufferIndex], tileIndex, weightStorage, weightAddress);
                loadVectorToShB(ptrB[bufferIndex], tileIndex, subgroupIndex, inputVector);

                // swap back to the previous buffer to perform the math.
                bufferIndex = bufferIndex ^ 1;
            }
            else
            {
                loadShA<U, Storage>(ptrA[0], tileIndex, weightStorage, weightAddress);
                loadVectorToShB(ptrB[0], tileIndex, subgroupIndex, inputVector);
                // For single buffer, we always need to sync the workgroup so the data is ready for all threads.
                // The reason that we have to sync the whole workgroup is that for matrix A, the tile is shared by
                // all workgroups.
                // The alternative solution to increase the performance is that we can duplicate the tile for each
                // subgroup, so each subgroup can perform it's own MMA operatiion, because we matrix B is only shared
                // by each subgroup. However, this solution will waste lots of shared memory as there are up to 32 subgroups
                // in the workgroup.
                GroupMemoryBarrierWithGroupSync();
            }

            // Math loop: This operator is executed by each warp (sub-group). shA is shared by all threads in the workgroup, while
            // shB is shared only by each subgroup, and each subgroup will have its own offset on shB.
            // For matB, each warp could only have 1 or 2 Tiles to load according whether this warp is less (1 tile) or more than half warp (2 tiles).
            MatA matA[NCoopMatRow];
            for (uint i = 0; i < NCoopMatRow; i ++)
            {
                if (TransposeA)
                {
                    // Really be careful the stride of tile A in the transpose case. The tile is column major, and the cooperative matrix is stacked in columns of the tile.
                    // So the stride is actually the height the tile. The offset between two cooperative matrices is just height of the cooperative matrix.
                    matA[i] = MatA.Load<ColumnMajor, uint4>(ptrA[bufferIndex] + i * T_ROW_A/ElementCountPerVector, HeightInVectorTileA);
                }
                else
                {
                    matA[i] = MatA.Load<RowMajor, uint4>(ptrA[bufferIndex] + i * CoopMatASizeInVector, WidthInVectorTileA);
                }
            }

            for (uint j = 0; j < NCoopMatColumn; j ++)
            {
                // NTilesColumn * HeightInVectorTileB is the size of the shared memory for matrix B in one warp measured in vector uint4.
                SPtr<uint4> ptrPerWarp = ptrB[bufferIndex] + subgroupIndex * SharedMemSizeInVectorMatB;
                let matB = MatB.Load<ColumnMajor, uint4>(ptrPerWarp + j * CoopMatBSizeInVector, HeightInVectorTileB);

                for (uint i = 0; i < NCoopMatRow; i ++)
                {
                    matC[i * NCoopMatColumn + j] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, T_ROW_A, T_COLUMN_A, T_COLUMN_B
                                            >(matA[i], matB, matC[i * NCoopMatColumn + j]);
                }
            }
        }

        // Write back the result to shared memory C. We store the result in column major because each column is
        // actually the output vector of the matrix-vector multiply, which is also a thread-local vector.
        // TODO: First, get the offset of C for each warp.
        OutArrayType outputVector;

        // Get start address of the shared memory for current warp.
        SPtr<uint4> ptrPerWarp = ptrC + subgroupIndex * SharedMemSizeInVectorMatC;
        for (int i = 0; i < NCoopMatRow; i ++)
        {
            for (int j = 0; j < NCoopMatColumn; j ++)
            {
                matC[i * NCoopMatColumn + j].Store<ColumnMajor, uint4>(ptrPerWarp + j * CoopMatCSizeInVector, T_ROW_C/ElementCountPerVector);
            }
            // Store one row of tiled result matrix back to the thread-local vector.
            storeVectorFromShB(ptrC, i, subgroupIndex, outputVector);

            // Wave sync is good enough, because shared memory C is only shared by each subgroup.
            GroupMemoryBarrierWithWaveSync();
        }
        return outputVector;
    }
}


// Cooperative matrix is only supported by CUDA and SPIR-V
[require(cooperative_matrix, subgroup_basic)]
public struct AccelerateVectorCoopMat<T, int N, int SubgroupSize> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = AccelerateVectorCoopMat<T.Differential, N, SubgroupSize>;

    public static const int Size = N;

    public no_diff SPtr<uint4> sharedMemoryPtr;

    internal static const uint ElementCountPerVector = sizeof(uint4) / sizeof(T);
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
            if (i < N)
            {
                this.data[i] = value;
            }
            else
            {
                this.data[i] = T(0.0f);
            }
        }
    }

    public __init(T[N] data)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            if (i < N)
            {
                this.data[i] = data[i];
            }
            else
            {
                this.data[i] = T(0.0f);
            }
        }
    }

    public __init(T[Uint4AlignedInputSize] data)
    {
        this.data = data;
    }

    public __init(This other) { this.data = other.data; }

    public __init<InputArray : IArray<T>>(InputArray data)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            this.data[i] = data[i];
    }

    public __subscript(int index) -> T
    {
        get { return data[index]; }
        set { data[index] = newValue; }
    }

    private OutputVector linearTransformOnTarget<int OutputSize, Storage, OutputVector, TargetEnum Target>(
        Storage weightStorage,
        no_diff Storage.Address weightAddress)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address
        where OutputVector : IVector<T, OutputSize>
    {
        typealias MMA = MMAHelper<T, N, OutputSize, SubgroupSize, Target, false>;
        SPtr<uint4> shA = sharedMemoryPtr;
        SPtr<uint4> shB = shA + MMA.SharedMemSizeInVectorMatA;
        SPtr<uint4> shC = shB;
        const int AlignedOutSize = MMA.Uint4AlignedM;
        let outputArray = MMA.mma<T, Storage, T[Uint4AlignedInputSize], T[AlignedOutSize]>( data, shA, shB, shC, weightStorage, weightAddress);
        return OutputVector(outputArray);
    }

    private static void linearTransformBwdOnTarget<int OutputSize, Storage, OutputVector, TargetEnum Target>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dWeightStorage,
        no_diff Storage.Address dWeightAddress,
        OutputVector.Differential doutput)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address
        where OutputVector : IVector<T, OutputSize>
        where OutputVector.Differential : IVector<T.Differential, OutputSize>
    {
        typealias MMA = MMAHelper<T.Differential, N, OutputSize, SubgroupSize, Target, true>;

        SPtr<uint4> shA = dthis.p.sharedMemoryPtr;
        SPtr<uint4> shB = shA + MMA.SharedMemSizeInVectorMatA;
        SPtr<uint4> shC = shB;

        // In backward, output size is K dimension.
        const int AlignedOutSize = MMA.Uint4AlignedK;
        const int AlignedInputSize = MMA.Uint4AlignedM;

        // This.Differential is the derivative of the input vector, which is the output
        // of the mma operation.
        // This.Differential outArray;

        // dIn = W^T * dOut;
#if 1
        let outArray = MMA.mma<T, Storage, OutputVector.Differential, T[AlignedOutSize].Differential
                        >( doutput, shA, shB, shC, dWeightStorage.p, dWeightAddress);
        This.Differential dInput = This.Differential(outArray);
        dthis = DifferentialPair<This>(dthis.p, dInput);

        MMA.outerProductAccumulate<T.Differential, Storage.Differential,
                                   T.Differential, OutputVector.Differential,
                                   T, T[Uint4AlignedInputSize]
                                  >( shA, shB, doutput, dthis.p.data, dWeightStorage.d, dWeightAddress);
#else
        let dInput = MMA.mma<T, Storage, OutputVector.Differential, This.Differential
                        >( doutput, shA, shB, shC, dWeightStorage.p, dWeightAddress);
        dthis = DifferentialPair<This>(dthis.p, dInput);

        MMA.outerProductAccumulate< T.Differential, Storage.Differential,
                                    T.Differential, OutputVector.Differential,
                                    T, This
                                  >( shA, shB, doutput, dthis.p, dWeightStorage.d, dWeightAddress);
#endif
    }

    // Linear transformation without bias
    [Differentiable]
    [BackwardDerivative(linearTransformBwd)]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        no_diff Storage.Address weightAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        __target_switch
        {
        case cuda:
            return no_diff linearTransformOnTarget<OutputSize, Storage, OutputVector, TargetEnum.CUDA>(weightStorage, weightAddress);
        case spirv:
            return no_diff linearTransformOnTarget<OutputSize, Storage, OutputVector, TargetEnum.SPIR_V>(weightStorage, weightAddress);
        }
    }

    // Backward of linear transformation without bias
    static void linearTransformBwd<int OutputSize, Storage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dWeightStorage,
        no_diff Storage.Address dWeightAddress,
        OutputVector.Differential doutput)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
            where OutputVector.Differential : IVector<T.Differential, OutputSize>
    {
        __target_switch
        {
        case cuda:
            linearTransformBwdOnTarget<OutputSize, Storage, OutputVector, TargetEnum.CUDA>(dthis, dWeightStorage, dWeightAddress, doutput);
        case spirv:
            linearTransformBwdOnTarget<OutputSize, Storage, OutputVector, TargetEnum.SPIR_V>(dthis, dWeightStorage, dWeightAddress, doutput);
        }
    }

    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        Storage biasStorage,
        no_diff Storage.Address weightAddress,
        no_diff Storage.Address biasAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();
        return output;
    }

    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(
        Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();
        return output;
    }

    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Address, OutputVector>(
        Address weightAddress, Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();
        return output;
    }
}
