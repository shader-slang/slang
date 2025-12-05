// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

#ifndef UNIT_TEST
#define VISIBILITY_LEVEL public
#else
#define VISIBILITY_LEVEL internal
#endif

internal interface CoopMatShape
{
    internal static const int ROW_A;
    internal static const int COLUMN_A;
    internal static const int ROW_B;
    internal static const int COLUMN_B;
    internal static const int ROW_C;
    internal static const int COLUMN_C;

    internal static const int SIZE_A;
    internal static const int SIZE_B;
    internal static const int SIZE_C;
}

// convenient extension to get the shape of the CoopMat A by just T.row and T.column
internal extension<T> T : CoopMatShape
    where T : __BuiltinFloatingPointType
{
    internal static const int ROW_A = ShapeSelector<T>()[0];    // M
    internal static const int COLUMN_A = ShapeSelector<T>()[2]; // K

    internal static const int ROW_B = ShapeSelector<T>()[2];    // K
    internal static const int COLUMN_B = ShapeSelector<T>()[1]; // N

    internal static const int ROW_C = ROW_A;
    internal static const int COLUMN_C = COLUMN_B;

    internal static const int SIZE_A = ROW_A * COLUMN_A;
    internal static const int SIZE_B = ROW_B * COLUMN_B;
    internal static const int SIZE_C = ROW_A * COLUMN_B;
}

// We will fix the cooperative matrix shape for matrix A, so if the data type is half, we will use 16x16.
// If the data type is int8/uint8, we will use 16x16 for CUDA and 16x32 for Vulkan.
[ForceInline]
internal int[3] ShapeSelector<T:__BuiltinFloatingPointType>()
{
    if (sizeof(T) == 2)
        return {16, 16, 16};
    else if (sizeof(T) == 1)
    {
        __target_switch
        {
        case cuda:
            return {16, 16, 16};
        case spirv:
            return {16, 32, 16};
        }
    }
    else
    {
        static_assert(false, "Only support half and int8/uint8 for matrix A/B");
        return {0, 0};
    }
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

// We can't convert T* to uint4* on shared memory in slang, therefore, we will provide two versions of shared memory pointer
// and implement both of them, and document about the difference between them.
internal typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

// A wrapper of matrix type to access each tile of the matrix
// Note: The reason we want to make SubgroupSize as a generic parameter instead of using WaveGetLaneCount() is that
//       we need the logic to be expand as much as possible during compile time. WaveGetLaneCount() is a runtime function,
//       so if we use it directly in our mma function, some of branches won't be eliminated during compile time.
[require(cooperative_matrix)]
VISIBILITY_LEVEL struct MMAHelper< T : __BuiltinFloatingPointType, int InputSize, int OutputSize, int SubgroupSize>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    // Weight matrix is in shape of M x K
    // Input vector will be collected by each thread in a sub-group to form a matrix in shape of K x N
    // Note, M, K, N are measured in T.
    // Since Cooperative matrix is only worked for a whole subgroup (warp), so we always use the SubgroupSize as the N value.
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    // Because the vectorized load is along K dimension (row of A and column of B), so we need to check
    //  if K is aligned with the vector size (ElementCountPerVector), we can use the vectorized load,
    // otherwise, we need to use the scalar load.
    // ElementCountPerVector is measured in T.
    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(T);
    static const bool IsAlignedVector = (K % ElementCountPerVector) == 0;

    static const int NTilesRow = (M + T.ROW_A - 1) / T.ROW_A;

    // Notice that because of our specific workload, NTilesColumn is always warp-size (as long as it's more than half warp) or half warp (<= half warp).
    static const int NTilesColumn = (N + T.COLUMN_B - 1) / T.COLUMN_B;
    static const int NTiles = NTilesRow * NTilesColumn;

    // M dimension is aligned with T.ROW_A.
    // N dimension is aligned with T.COLUMN_B.
    // Measured in T.
    static const int AlignedM = NTilesRow * T.ROW_A;
    static const int AlignedN = NTilesColumn * T.COLUMN_B;

    // `Uint4AlignedK` is the K value that is aligned with the uint4 vector size. Because we always do the vectorized load
    // along K dimension, so we need to align the K value with the vector size.
    // Measured in T.
    VISIBILITY_LEVEL static const int Uint4AlignedK = ((K + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;
    VISIBILITY_LEVEL static const int Uint4AlignedM = ((M + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;


    // In order to achieve the high throughput, we will write data to shared memory in vectorized way.
    // e.g. if T is half, it's 2 bytes, so uint4 can store 8 elements.
    // Since the size in bytes of the tile in one row is T.COLUMN * sizeof(T),
    // the number of vectorized columns in one row of cooperative matrix A is T.COLUMN * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4.
    static const uint WidthInVectorTileA = T.COLUMN_A / ElementCountPerVector;
    // WidthInElementTileA is the number of elements in one row of the tile in original data type.
    static const uint WidthInElementTileA = WidthInVectorTileA * ElementCountPerVector;

    // Similarly, the number of vectorized rows in one column of cooperative matrix B is T.ROW * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    static const uint HeightInVectorTileB = T.ROW_B / ElementCountPerVector;

    // So HeightInElementTileB is the number of elements in one column of the tile, measured in T.
    static const uint HeightInElementTileB = HeightInVectorTileB * ElementCountPerVector;

    // The number of vectorized rows in one column of Cooperative matrix C.
    static const uint HeightInVectorTileC = T.ROW_C / ElementCountPerVector;

    // The aligned size of the shared memory for matrix A and B.
    // The size of the shared memory is the tile size for each matrices, measured in uint4.
    // Tile A = AlignedM * Tile_width of A
    // Tile B = Tile_height of B * AlignedN
    // Tile C = Tile_height of B * AlignedN
    // Note B and C are the same size because their widths are always the same as subGroupSize and
    // their heights are fixed determined by Cooperative matrix shape.
    static const uint SharedMemSizeInVectorMatA = (AlignedM * T.COLUMN_A) / ElementCountPerVector;
    static const uint SharedMemSizeInVectorMatB = (T.ROW_B * AlignedN) / ElementCountPerVector;
    static const uint SharedMemSizeInVectorMatC = SharedMemSizeInVectorMatB;

    static const uint CoopMatASizeInVector = T.SIZE_A / ElementCountPerVector;
    static const uint CoopMatBSizeInVector = T.SIZE_B / ElementCountPerVector;
    static const uint CoopMatCSizeInVector = T.SIZE_C / ElementCountPerVector;


    // NumLoadsEachThreadMatA is the number of loads each thread needs to perform.
    // N is the number of active threads in the subgroup.
    // For matrix A, each iteration we load a column of matrix tile of Matrix A, which is M x 32 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to
    // (M * 32) / (N * 16) = M * 2 / N, and we just round up the result to the nearest integer.
    static const uint NumLoadsEachThreadMatA = (AlignedM * 2 + N - 1) / N;

    // OffsetPerThreadLoadMatA is the offset between each load.
    // The reason is same as above, since the whole subgroup can load N * 16 bytes, and the load is consecutive,
    // so the offset is the N * 16 bytes, and we measure it in uint4 units, so it's N.
    static const uint OffsetPerThreadLoadMatA = N;

    // A/B can only be half, so we don't need to use generic parameter here.
    typealias MatA = linalg.CoopMat< half, MemoryScope.Subgroup, T.ROW_A, T.COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat< half, MemoryScope.Subgroup, T.COLUMN_A, T.COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T, MemoryScope.Subgroup, T.ROW_A, T.COLUMN_B, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    static const bool EnableDoubleBuffer = false;

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
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void LoadShA<Storage>(
            SPtr<uint4> sharedMemoryA,
            int tileIndex,
            Storage weightStorage,
            Storage.Address weightAddress)
            where Storage : IStorage<T>
    {
        const uint laneId = WaveGetLaneIndex();

        // get the 2-D coordinate of the tile in the vectorized tile
        const uint xInTile = laneId % WidthInVectorTileA;
        const uint yInTile = laneId / WidthInVectorTileA;

        // tx and ty are the 2-D coordinate of scalar tile.
        // row coordinate ty is the same between vectorized tile and original tile.
        // column coordinate tx needs to be scaled by ElementCountPerVector to get the actual column coordinate in original data type.
        uint tx = xInTile * ElementCountPerVector + tileIndex * WidthInElementTileA;
        uint ty = yInTile;
        uint indexInVectorizedTile = yInTile * WidthInVectorTileA + xInTile;

        uint4 value;
        uint indexInWeight;
        for (uint i = 0; i < NumLoadsEachThreadMatA; i++)
        {
            if (ty >= AlignedM)
                break;
            // Given the coordinate inside the tile and the tile index, we can get the index in weight matrix.
            // Note, we always treat row of matrix A as uint4 aligned, so always calculate 'indexInWeight' as if it's uint4 aligned.
            // though it might not. But IStorage.readUint4() will handle the padding.
            // We only need to provide the actual boundary and the aligned boundary.
            indexInWeight = ty * K + tx;

            // Bounds check. If the coordinate is out of range of Matrix A, just padding with 0.
            bool isOutOfRange = ((ty >= M) | (tx >= K));
            if (isOutOfRange)
            {
                value = uint4(0, 0, 0, 0);
            }
            else
            {
                let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
                // Though the weight matrix is not necessarily aligned with the uint4 vector size, readUnit4() will handle the padding
                // if the reading is cross the boundary of the uint4 vector size.
                value = weightStorage.readUint4<IsAlignedVector, K, Uint4AlignedK>(weightAddress, offsetAddress);
            }

            sharedMemoryA[indexInVectorizedTile] = value;
            indexInVectorizedTile += OffsetPerThreadLoadMatA;

            // Given the index in the tile, get the 2-D coordinate in the matrix A.
            tx = (indexInVectorizedTile % WidthInVectorTileA) * ElementCountPerVector + tileIndex * WidthInElementTileA;
            ty = indexInVectorizedTile / WidthInVectorTileA;
        }
    }

    // Load the input vector into shared memory. Each input vector is a column of the matrix B.
    // Since the input vector is loaded from thread local memory, one thread can only load one
    // column of the matrix B.
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void LoadSharedMemoryFromLocalVector(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        T[Uint4AlignedK] inputVector)
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
            if (startIndex >= K)
            {
                sharedMemoryBSubgroup[indexInVectorizedTile + yInTile] = uint4(0, 0, 0, 0);
                continue;
            }

            // It's fine that we only used the aligned version of readUint4() here, because the inputVector is the internal data which we can always
            // create it as an aligned array.
            uint4 value = readUint4<T>(inputVector, startIndex);
            sharedMemoryBSubgroup[indexInVectorizedTile + yInTile] = value;
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
    VISIBILITY_LEVEL static void StoreSharedMemoryToLocalVector(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        int subgroupIndex,
        out T[Uint4AlignedM] outputVector)
    {
        const uint laneId = WaveGetLaneIndex();
        SPtr<uint4> sharedMemoryBSubgroup = sharedMemoryB + subgroupIndex * SharedMemSizeInVectorMatB;

        // start index in vectorized tile for current thread: x * column_stride
        const uint indexInVectorizedTile = laneId * HeightInVectorTileB;
        const uint yOffset = (tileIndex * HeightInVectorTileB) * ElementCountPerVector;

        for (uint yInTile = 0; yInTile < HeightInVectorTileB; yInTile++)
        {
            const int startIndex = yInTile * ElementCountPerVector + yOffset;

            // Bounds check on Y direction. If y coordinate out of the input vector length, just break.
            // Because we don't need padding when store the result back to thread-local vector.
            if (startIndex >= M)
            {
                break;
            }

            writeUint4<T>(outputVector, startIndex, sharedMemoryBSubgroup[indexInVectorizedTile + yInTile]);
        }

        // No padding needs for store to local vector. We need padding when loading from local vector to shared memory
        // because the alignment requirement for the shared memory. But when store it back to local vector, those padding
        // Columns are actually corresponding to the inactive threads, where there are no thread-local data at all.
    }

    // The workload is different from the large size matrix-matrix multiply, we actually perform the matrix-vector multiply
    //  for each thread, so the matrix-matrix multiply only needs to be performed for each warp (sub-group).
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static T[Uint4AlignedM] mma<Storage>( T[Uint4AlignedK] inputVector,
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        SPtr<uint4> sharedMemoryC,
        Storage weightStorage,
        Storage.Address weightAddress
        )
            where Storage : IStorage<T>
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
            LoadShA(ptrA[0], 0, weightStorage, weightAddress);
            LoadSharedMemoryFromLocalVector(ptrB[0], 0, subgroupIndex, inputVector);
        }

        // fetch first tile of Matrix A and Matrix B into shared memory
        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        uint bufferIndex = 0;

        MatC matC[NTilesRow * NTilesColumn];

        for (int i = 0; i < NTilesRow * NTilesColumn; i++)
        {
            matC[i].fill(T(0.0f));
        }

        for (int k = 0; k < Uint4AlignedK; k += T.COLUMN_A)
        {
            uint tileIndex = k / T.COLUMN_A;

            if (EnableDoubleBuffer)
            {
                // warp sync to ensure all threads in the warp have loaded the data.
                // The reason we always put
                GroupMemoryBarrierWithWaveSync();

                // swap to next buffer to load new data.
                bufferIndex = bufferIndex ^ 1;
                // Load the another buffer and do the math so we can hide the latency of the load.
                LoadShA(ptrA[bufferIndex], tileIndex, weightStorage, weightAddress);
                LoadSharedMemoryFromLocalVector(ptrB[bufferIndex], tileIndex, subgroupIndex, inputVector);

                // swap back to the previous buffer to perform the math.
                bufferIndex = bufferIndex ^ 1;
            }
            else
            {
                LoadShA(ptrA[0], tileIndex, weightStorage, weightAddress);
                LoadSharedMemoryFromLocalVector(ptrB[0], tileIndex, subgroupIndex, inputVector);
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
            MatA matA[NTilesRow];
            for (uint i = 0; i < NTilesRow; i ++)
            {
                matA[i] = MatA.Load<linalg.CoopMatMatrixLayout.RowMajor, uint4>(ptrA[bufferIndex] + i * CoopMatASizeInVector, T.COLUMN_A/ElementCountPerVector);
            }

            for (uint j = 0; j < NTilesColumn; j ++)
            {
                // NTilesColumn * HeightInVectorTileB is the size of the shared memory for matrix B in one warp measured in vector uint4.
                SPtr<uint4> ptrPerWarp = ptrB[bufferIndex] + subgroupIndex * SharedMemSizeInVectorMatB;
                let matB = MatB.Load<linalg.CoopMatMatrixLayout.ColumnMajor, uint4>(ptrPerWarp + j * CoopMatBSizeInVector, T.ROW_B/ElementCountPerVector);

                for (uint i = 0; i < NTilesRow; i ++)
                {
                    matC[i * NTilesColumn + j] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, T.ROW_A, T.COLUMN_A, T.COLUMN_B
                                            >(matA[i], matB, matC[i * NTilesColumn + j]);
                }
            }
        }

        // Write back the result to shared memory C. We store the result in column major because each column is
        // actually the output vector of the matrix-vector multiply, which is also a thread-local vector.
        // TODO: First, get the offset of C for each warp.
        // ptrCPerWarp = ptrC + subgroupIndex * (subGroupSize * AlignedM / ElementCountPerVector);
        T[Uint4AlignedM] outputVector;
        // Get start address of the shared memory for current warp.
        SPtr<uint4> ptrPerWarp = ptrC + subgroupIndex * SharedMemSizeInVectorMatC;
        for (int i = 0; i < NTilesRow; i ++)
        {
            for (int j = 0; j < NTilesColumn; j ++)
            {
                matC[i * NTilesColumn + j].Store<linalg.CoopMatMatrixLayout.ColumnMajor, uint4>(ptrPerWarp + j * CoopMatCSizeInVector, T.ROW_C/ElementCountPerVector);
            }
            // Store one row of tiled result matrix back to the thread-local vector.
            StoreSharedMemoryToLocalVector(ptrC, i, subgroupIndex, outputVector);

            // Wave sync is good enough, because shared memory C is only shared by each subgroup.
            GroupMemoryBarrierWithWaveSync();
        }
        return outputVector;
    }
}

// Cooperative matrix is only supported by CUDA and SPIR-V
[require(cuda_spirv)]
public struct AccelerateVectorCoopMat<T, int N, int SubgroupSize> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = AccelerateVectorCoopMat<T.Differential, N, SubgroupSize>;

    public static const int Size = N;

    // We will collect the vector by each thread in a sub-group to form a cooperative matrix for acceleration,
    // the vector size is one dimension of the cooperative matrix, and the other dimension is the batch dimension.
    internal static const int batchDimension = WaveGetLaneCount();

    internal static const int Row = batchDimension;
    internal static const int Column = N;

    SPtr<uint4> sharedMemoryPtr;

    [DerivativeMember(Differential.data)]
    internal T[N] data;

    public __init() { data = {}; }

    public __init(T value)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            data[i] = value;
    }

    public __init(T[N] data) { this.data = data; }

    public __init(This other) { data = other.data; }

    public __subscript(int index) -> T
    {
        get { return data[index]; }
        set { data[index] = newValue; }
    }

    // Linear transformation without bias
    [Differentiable]
    public OutputVector linearTransform<int OutputSize, Storage, OutputVector>(
        Storage weightStorage,
        no_diff Storage.Address weightAddress)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();
        return output;
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
