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
        uint3 blockIdx = cudaBlockIdx();
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

// convenient extension to get the shape of the CoopMat A by just T.row and T.column
internal extension<T> T: CoopMatShape
    where T : __BuiltinFloatingPointType
{
    internal static const int ROW_A = ShapeSelector<T>()[0];        // M
    internal static const int COLUMN_A = ShapeSelector<T>()[2];     // K

    internal static const int ROW_B = ShapeSelector<T>()[2];        // K
    internal static const int COLUMN_B = ShapeSelector<T>()[1];     // N
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
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    static const int ElementCountPerRowA = K;

    static const int NTilesRow    = (M + T.ROW_A - 1) / T.ROW_A;
    static const int NTilesColumn = (N + T.COLUMN_B - 1) / T.COLUMN_B;

    static const int NTiles = NTilesRow * NTilesColumn;

    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(T);

    // In order to achieve the high throughput, we will write data to shared memory in vectorized way.
    // e.g. if T is half, it's 2 bytes, so uint4 can store 8 elements.
    // Since the size in bytes of the tile in one row is T.COLUMN * sizeof(T),
    // so the number of vectorized columns in one row is T.COLUMN * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4.
    static const uint WidthInVectorTileA = T.COLUMN_A * sizeof(T) / sizeof(uint4);
    // So TileWidthAInElement is the number of elements in one row of the tile in original data type.
    static const uint WidthInElementTileA = WidthInVectorTileA * ElementCountPerVector;

    // Similarly, the number of vectorized rows in one column is T.ROW * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    // It's used for column major matrix B.
    static const uint HeightInVectorTileB = T.ROW_B * sizeof(T) / sizeof(uint4);

    // So TileHeightBInElement is the number of elements in one column of the tile in original data type.
    static const uint HeightInElementTileB = HeightInVectorTileB * ElementCountPerVector;

    // calculate that how many loads each thread needs to perform.
    // For matrix A, each iteration we load a column of matrix tile of Matrix A, which is M x 32 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to
    // perform is (M * 32) / (warpSize * 16) = M * 2 / warpSize.
    static const uint NumLoadsEachThreadMatA = M * 2 / SubgroupSize;

    // For each thread, if it can load more than one vectorized element, this is the offset between each load.
    // The reason is same as above, since the whole warp can load warpSize * 16 bytes, and the load is consecutive,
    // so the offset is the warp size * 16 bytes, and we measure it in uint4 units, so it's WaveGetLaneCount().
    static const uint OffsetPerThreadLoadMatA = SubgroupSize;
    static const uint DxPerThreadLoadMatA = OffsetPerThreadLoadMatA % WidthInVectorTileA;
    static const uint DyPerThreadLoadMatA = OffsetPerThreadLoadMatA / WidthInVectorTileA;

    static const uint SharedMemSizeInBytesMatA = M * (T.COLUMN_A * sizeof(T));
    static const uint SharedMemSizeInBytesMatB = N * (T.ROW_B * sizeof(T));

    // A/B can only be half, so we don't need to use generic parameter here.
    typealias MatA = linalg.CoopMat< half, MemoryScope.Subgroup, T.ROW_A, T.COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat< half, MemoryScope.Subgroup, T.COLUMN_A, T.COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T, MemoryScope.Subgroup, T.ROW_A, T.COLUMN_B, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    VISIBILITY_LEVEL static uint4 readUint4<T>(T[InputSize] inputVector, int index)
    {
        const int nBytes = sizeof(T);
        uint4 result = uint4(0, 0, 0, 0);
        switch (nBytes)
        {
        case 1:
            return uint4(
                uint(bit_cast<uint8_t>(inputVector[index])) << 24 |
                uint(bit_cast<uint8_t>(inputVector[index + 1])) << 16 |
                uint(bit_cast<uint8_t>(inputVector[index + 2])) << 8 |
                uint(bit_cast<uint8_t>(inputVector[index + 3])),

                uint(bit_cast<uint8_t>(inputVector[index + 4])) << 24 |
                uint(bit_cast<uint8_t>(inputVector[index + 5])) << 16 |
                uint(bit_cast<uint8_t>(inputVector[index + 6])) << 8 |
                uint(bit_cast<uint8_t>(inputVector[index + 7])),

                uint(bit_cast<uint8_t>(inputVector[index + 8])) << 24 |
                uint(bit_cast<uint8_t>(inputVector[index + 9])) << 16 |
                uint(bit_cast<uint8_t>(inputVector[index + 10])) << 8 |
                uint(bit_cast<uint8_t>(inputVector[index + 11])),

                uint(bit_cast<uint8_t>(inputVector[index + 12])) << 24 |
                uint(bit_cast<uint8_t>(inputVector[index + 13])) << 16 |
                uint(bit_cast<uint8_t>(inputVector[index + 14])) << 8 |
                uint(bit_cast<uint8_t>(inputVector[index + 15])),
        );

        case 2:
            return uint4(
                uint(bit_cast<uint16_t>(inputVector[index])) << 16 |
                uint(bit_cast<uint16_t>(inputVector[index + 1])),

                uint(bit_cast<uint16_t>(inputVector[index + 2])) << 16 |
                uint(bit_cast<uint16_t>(inputVector[index + 3])),

                uint(bit_cast<uint16_t>(inputVector[index + 4])) << 16 |
                uint(bit_cast<uint16_t>(inputVector[index + 5])),

                uint(bit_cast<uint16_t>(inputVector[index + 6])) << 16 |
                uint(bit_cast<uint16_t>(inputVector[index + 7])),
            );
        case 4:
            return uint4(
                bit_cast<uint>(inputVector[index]),
                bit_cast<uint>(inputVector[index + 1]),
                bit_cast<uint>(inputVector[index + 2]),
                bit_cast<uint>(inputVector[index + 3]),
            );
        }
        static_assert(false, "Unsupported data type T");
        return uint4(0);
    }

    [require(cooperative_matrix, subgroup_basic)]
    [ForceInline]
    VISIBILITY_LEVEL static void LoadShA<Storage>(
            SPtr<uint4> sharedMemoryA,
            int tileIndex,
            Storage weightStorage,
            Storage.Address weightAddress)
            where Storage : IStorage<T>
    {
        const uint landId = WaveGetLaneIndex();

        // get the 2-D coordinate of the tile in the vectorized tile
        const uint xInTile = landId % WidthInVectorTileA;
        const uint yInTile = landId / WidthInVectorTileA;

        // pre-load the Matrix A into shared memory.
        // We load a column of matrix tile of Matrix A each time, because our iteration is over K dimension.
        // We require the Matrix A is stored as row major in the weight storage.
        // Also we load the tile into shared memory in row major.
        // get the 2-D coordinate of the tile in original data type

        // tx and ty are the 2-D coordinate of scalar tile.
        // row coordinate ty is the same between vectorized tile and original tile.
        // column coordinate tx needs to be scaled by ElementCountPerVector to get the actual column coordinate in original data type.
        const uint tx = xInTile * ElementCountPerVector;
        const uint ty = yInTile;
        uint indexInVectorizedTile = yInTile * WidthInVectorTileA + xInTile;

        // given the coordinate inside the tile and the tile index, we can get the index in weight matrix.
        uint indexInWeight = ty * ElementCountPerRowA + tx + tileIndex * WidthInElementTileA;

        for (uint i = 0; i < NumLoadsEachThreadMatA; i++)
        {
            // Get the 1-D index in the weight matrix for current thread, note since this the first tile, so
            // there is no offset here. Otherwise, the offset will be the tile index.
            // TODO: Bounds check.
            let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
            uint4 value = weightStorage.readUint4(offsetAddress);

            sharedMemoryA[indexInVectorizedTile] = value;
            indexInVectorizedTile += OffsetPerThreadLoadMatA;

            // In 2-D space, [dx, dy] within the tile is same as [dx, dy] in weight matrix.
            // The only difference is that we need to measure the dx in length of element T.
            // So we need to multiply the ElementCountPerVector to get the actual dx.
            const uint dx = DxPerThreadLoadMatA * ElementCountPerVector;
            const uint dy = DyPerThreadLoadMatA;
            indexInWeight = indexInWeight + dy * ElementCountPerRowA + dx;
        }
    }

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void LoadSharedMemoryFromLocalVector(
        SPtr<uint4> sharedMemoryB,
        int tileIndex,
        T[K] inputVector)
    {
        const uint xInTile = WaveGetLaneIndex();

        // start index in vectorized tile for current thread: x * column_stride
        const uint indexInVectorizedTile = xInTile * HeightInVectorTileB;
        const uint yOffset = (tileIndex * HeightInVectorTileB) * ElementCountPerVector;

        for (uint yInTile = 0; yInTile < HeightInVectorTileB; yInTile++)
        {
            // TODO: Bounds check.
            uint4 value = readUint4(inputVector, yInTile * ElementCountPerVector + yOffset);
            sharedMemoryB[indexInVectorizedTile + yInTile] = value;
        }
    }

    // The workload is different from the large size matrix-matrix multiply, we actually perform the matrix-vector multiply
    //  for each thread, so the matrix-matrix multiply only needs to be performed for each warp (sub-group).
    [require(cooperative_matrix, subgroup_basic)]
    VISIBILITY_LEVEL static void mma<Storage>( T[K] inputVector,
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        Storage weightStorage,
        Storage.Address weightAddress)
            where Storage : IStorage<T>
    {
        // lane id in a sub-group
        const uint landId = WaveGetLaneIndex();
        // get the 2-D coordinate of the tile in the vectorized tile
        const uint xInTile = landId % WidthInVectorTileA;
        const uint yInTile = landId / WidthInVectorTileA;

        // pre-load the Matrix A into shared memory.
        // We load a column of matrix tile of Matrix A each time, because our iteration is over K dimension.
        // We require the Matrix A is stored as row major in the weight storage.
        // Also we load the tile into shared memory in row major.
        // get the 2-D coordinate of the tile in original data type
        // tx and ty are the 2-D coordinate of scalar
        const uint tx = xInTile * ElementCountPerVector;
        const uint ty = yInTile;

        uint indexInVectorizedTile = yInTile * WidthInVectorTileA + xInTile;
        uint indexInWeight = ty * ElementCountPerRowA + tx;

        for (uint i = 0; i < NumLoadsEachThreadMatA; i++)
        {
            // Get the 1-D index in the weight matrix for current thread, note since this the first tile, so
            // there is no offset here. Otherwise, the offset will be the tile index.
            // TODO: Bounds check.
            let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
            uint4 value = weightStorage.readUint4(offsetAddress);

            sharedMemoryA[indexInVectorizedTile] = value;
            indexInVectorizedTile += OffsetPerThreadLoadMatA;

            // Given the index in tile, we need to get the index in weight matrix
            // newTx and newTy are the x and y coordinates in tile but but measured by element T.
            // convert offset into 2-D coordinate in tile. So we can use this dx and dy to get
            // the offset in weight matrix. Because on 2-D, [dx, dy] within the tile is same as
            // [dx, dy] in weight matrix. The only difference is that we need to measure the dx
            // in length of T. So we need to multiply the vectorizedColNum to get the actual dx.
            const uint dx = (DxPerThreadLoadMatA % WidthInVectorTileA) * ElementCountPerVector;
            const uint dy = DyPerThreadLoadMatA * WidthInVectorTileA;
            indexInWeight = indexInWeight + dy * ElementCountPerRowA + dx;
        }

        // pre-load the Matrix B into shared memory.
        // Since we use subgroup size as N, it's natural that the Matrix B is stored as column major,
        // as each column can only be collected by one thread in the subgroup.
        // We load a row of matrix tile of Matrix B each time, so the iteration is vectorized Tile Height.
        // TODO: If less than subgroup size threads are launched, then there are no other threads in a warp can fill the shared
        // memory for Matrix B. We need to handle this case as well. The same problem exists for Matrix A as well.

        indexInVectorizedTile = xInTile * HeightInVectorTileB;
        for (uint i = 0; i < HeightInVectorTileB; i++)
        {
            // TODO: Bounds check.
            uint4 value = readUint4(inputVector, i);
            sharedMemoryB[indexInVectorizedTile + i] = value;
        }

        // fetch first tile of Matrix A and Matrix B into shared memory
        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        uint doubleBufferSwitch = 0;
        MatC[NTilesRow][NTilesColumn] matC;
        for (int i = 0; i < NTilesRow; i ++)
        {
            for (int j = 0; j < NTilesColumn; j ++)
            {
                matC[i][j].fill(T(0.0f));
            }
        }

        for (int k = 0; k < K; k += T.COLUMN_A)
        {
            // warp sync to ensure all threads in the warp have loaded the data.
            AllMemoryBarrierWithWaveSync();

            // switch to another buffer
            doubleBufferSwitch = doubleBufferSwitch ^ 1; // flip between 0 and 1
            uint sharedMemOffsetMatA = doubleBufferSwitch * SharedMemSizeInBytesMatA;
            uint sharedMemOffsetMatB = doubleBufferSwitch * SharedMemSizeInBytesMatB;
            SPtr<uint4> shMemPtrMatA = sharedMemoryA + sharedMemOffsetMatA / sizeof(uint4);
            SPtr<uint4> shMemPtrMatB = sharedMemoryB + sharedMemOffsetMatB / sizeof(uint4);

            // TODO: Load the another buffer and do the math so we can hide the latency of the load.

            uint indexInVectorizedTile = yInTile * WidthInVectorTileA + xInTile;

            // k is the x-coordinate of tile id, so (xInTile + k * vectorizedTileWidthA) is the x-coordinate in weight matrix,
            // but measured by vector uint4, so multiply elementCountPerVector to get the actual x-coordinate measured by element T.
            uint indexInWeight = yInTile * ElementCountPerRowA + (xInTile + k * WidthInVectorTileA) * ElementCountPerVector;
            uint offset = 0;
            for (uint i = 0; i < NumLoadsEachThreadMatA; i++)
            {
                // Get the 1-D index in the weight matrix for current thread, note since this the first tile, so
                // there is no offset here. Otherwise, the offset will be the tile index.
                // TODO: Bounds check.
                let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
                uint4 value = weightStorage.readUint4(offsetAddress);

                shMemPtrMatA[indexInVectorizedTile] = value;
                indexInVectorizedTile += OffsetPerThreadLoadMatA;

                // Given the index in tile, we need to get the index in weight matrix
                // newTx and newTy are the x and y coordinates in tile but but measured by element T.
                // convert offset into 2-D coordinate in tile. So we can use this dx and dy to get
                // the offset in weight matrix. Because on 2-D, [dx, dy] within the tile is same as
                // [dx, dy] in weight matrix. The only difference is that we need to measure the dx
                // in length of T. So we need to multiply the vectorizedColNum to get the actual dx.
                const uint dx = (DxPerThreadLoadMatA % WidthInVectorTileA) * ElementCountPerVector;
                const uint dy = DyPerThreadLoadMatA * WidthInVectorTileA;
                indexInWeight = indexInWeight + dy * ElementCountPerRowA + dx;
            }

            indexInVectorizedTile = xInTile * HeightInVectorTileB;
            for (uint i = 0; i < HeightInVectorTileB; i++)
            {
                // TODO: Bounds check.
                uint4 value = readUint4(inputVector, i);
                sharedMemoryB[indexInVectorizedTile + i] = value;
            }

            // Math loop
            // switch to previous buffer to perform the math.
            doubleBufferSwitch = doubleBufferSwitch ^ 1; // flip between 0 and 1
            sharedMemOffsetMatA = doubleBufferSwitch * SharedMemSizeInBytesMatA;
            sharedMemOffsetMatB = doubleBufferSwitch * SharedMemSizeInBytesMatB;
            shMemPtrMatA = sharedMemoryA + sharedMemOffsetMatA / sizeof(uint4);
            shMemPtrMatB = sharedMemoryB + sharedMemOffsetMatB / sizeof(uint4);

            // TODO: rounding
            MatA matA[NTilesRow];
            for (int i = 0; i < NTilesRow; i ++)
            {
                int stride = WidthInVectorTileA * sizeof(uint4);
                matA[i].Load<linalg.CoopMatMatrixLayout.RowMajor, uint4>(shMemPtrMatA + i * T.ROW_A * stride, stride);
            }

            MatB matB;
            for (int j = 0; j < NTilesColumn; j ++)
            {
                int stride = HeightInVectorTileB * sizeof(uint4);
                matB.Load<linalg.CoopMatMatrixLayout.ColumnMajor, uint4>(shMemPtrMatB + j * T.COLUMN_B * stride, stride);

                for (uint i = 0; i < NTilesRow; i ++)
                {
                    matC[i][j] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, T.ROW_A, T.COLUMN_A, T.COLUMN_B
                                            >(matA[i], matB, matC[i][j]);
                }
            }
        }
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

//    internal uint4[(N + 3) / 4] highThroughputData;

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
