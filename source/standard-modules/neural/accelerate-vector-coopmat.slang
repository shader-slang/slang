implementing neural;

internal interface IMatrixAShape
{
    internal static const int ROW;
    internal static const int COLUMN;
}

// We will fix the cooperative matrix shape for matrix A, so if the data type is half, we will use 16x16.
// If the data type is int8/uint8, we will use 16x16 for CUDA and 16x32 for Vulkan.
[ForceInline]
internal int[2] ShapeSelector<T:__BuiltinFloatingPointType>()
{
    if (sizeof(T) == 2)
        return {16, 16};
    else if (sizeof(T) == 1)
    {
        __target_switch
        {
        case cuda:
            return {16, 16};
        case spirv:
            return {16, 32};
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
internal extension<T> T: IMatrixAShape
    where T : __BuiltinFloatingPointType
{
    internal static const int ROW = ShapeSelector<T>()[0];
    internal static const int COLUMN = ShapeSelector<T>()[1];
}

// We can't convert T* to uint4* on shared memory in slang, therefore, we will provide two versions of shared memory pointer
// and implement both of them, and document about the difference between them.
internal typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

// A wrapper of matrix type to access each tile of the matrix
[require(cuda_spirv)]
internal struct MMAHelper<T : __BuiltinFloatingPointType, int InputSize, int OutputSize>
{
    // Weight matrix is in shape of M x K
    // Input vector will be collected by each thread in a sub-group to form a matrix in shape of K x N
    static const int M = OutputSize;
    static const int N = WaveGetLaneCount();
    static const int K = InputSize;

    static const int ElementCountPerRowA = K * sizeof(T);

    static const int nTilesRow = (M + T.ROW - 1) / T.ROW;
    static const int nTilesColumn = (N + T.COLUMN - 1) / T.COLUMN;

    static const int nTiles = nTilesRow * nTilesColumn;

    static const uint elementCountPerVector = sizeof(uint4) / sizeof(T);

    // In order to achieve the high throughput, we will write data to shared memory in vectorized way.
    // e.g. if T is half, it's 2 bytes, so uint4 can store 8 elements.
    // Since the size in bytes of the tile in one row is T.COLUMN * sizeof(T),
    // so the number of vectorized columns in one row is T.COLUMN * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    static const uint vectorizedTileWidthA = T.COLUMN * sizeof(T) / sizeof(uint4);

    // Similarly, the number of vectorized rows in one column is T.ROW * sizeof(T) / sizeof(uint4).
    // `1` means one element of uint4
    // It's used for column major matrix B.
    static const uint vectorizedTileHeightB = T.ROW * sizeof(T) / sizeof(uint4);


    // calculate that how many loads each thread needs to perform.
    // For matrix A, each iteration we load a column of matrix tile of Matrix A, which is M x 32 bytes.
    // And each thread can load 16 bytes for each load, so the number of loads each thread needs to
    // perform is (M * 32) / (warpSize * 16) = M * 2 / warpSize.
    static const uint numLoadsEachThreadMatA = M * 2 / WaveGetLaneCount();

    // For each thread, if it can load more than one vectorized element, this is the offset between each load.
    // The reason is same as above, since the whole warp can load warpSize * 16 bytes, and the load is consecutive,
    // so the offset is the warp size * 16 bytes, and we measure it in uint4 units, so it's WaveGetLaneCount().
    static const uint offsetPerThreadLoadMatA = WaveGetLaneCount();
    static const uint dxPerThreadLoadMatA = offsetPerThreadLoadMatA % vectorizedTileWidthA;
    static const uint dyPerThreadLoadMatA = offsetPerThreadLoadMatA / vectorizedTileWidthA;

    static const uint sharedMemSizeInBytesA = M * (T.COLUMN * sizeof(T));
    static const uint sharedMemSizeInBytesB = N * (T.ROW * sizeof(T));

    // A/B can only be half, so we don't need to use generic parameter here.
    typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, T.ROW, T.COLUMN, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, T.ROW, T.COLUMN, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat< T, MemoryScope.Subgroup, T.ROW, T.COLUMN, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    private uint4 readUint4<T>(T[K] inputVector, int index)
        where T : __BuiltinFloatingPointType
        where T.Differential == T
    {
        const int nBytes = sizeof(T);
        uint4 result = uint4(0, 0, 0, 0);
        switch (nBytes)
        {
        case 1:
            return uint4(
                bit_cast<uint>(inputVector[index]) << 24 |
                bit_cast<uint>(inputVector[index + 1]) << 16 |
                bit_cast<uint>(inputVector[index + 2]) << 8 |
                bit_cast<uint>(inputVector[index + 3]),

                bit_cast<uint>(inputVector[index + 4]) << 24 |
                bit_cast<uint>(inputVector[index + 5]) << 16 |
                bit_cast<uint>(inputVector[index + 6]) << 8 |
                bit_cast<uint>(inputVector[index + 7]),

                bit_cast<uint>(inputVector[index + 8]) << 24 |
                bit_cast<uint>(inputVector[index + 9]) << 16 |
                bit_cast<uint>(inputVector[index + 10]) << 8 |
                bit_cast<uint>(inputVector[index + 11]),

                bit_cast<uint>(inputVector[index + 12]) << 24 |
                bit_cast<uint>(inputVector[index + 13]) << 16 |
                bit_cast<uint>(inputVector[index + 14]) << 8 |
                bit_cast<uint>(inputVector[index + 15]),
        );

        case 2:
            return uint4(
                bit_cast<uint>(inputVector[index]) << 16 |
                bit_cast<uint>(inputVector[index + 1]),

                bit_cast<uint>(inputVector[index + 2]) << 16 |
                bit_cast<uint>(inputVector[index + 3]),

                bit_cast<uint>(inputVector[index + 4]) << 16 |
                bit_cast<uint>(inputVector[index + 5]),

                bit_cast<uint>(inputVector[index + 6]) << 16 |
                bit_cast<uint>(inputVector[index + 7]),
            );
        case 4:
            return uint4(
                bit_cast<uint>(inputVector[index]),
                bit_cast<uint>(inputVector[index + 1]),
                bit_cast<uint>(inputVector[index + 2]),
                bit_cast<uint>(inputVector[index + 3]),
            );
        }
    }


    // The workload is different from the large size matrix-matrix multiply, we actually perform the matrix-vector multiply
    // for each thread, so the matrix-matrix multiply only needs to be performed for each warp (sub-group).
    void mma<T, Storage>(T[K] inputVector,
        SPtr<uint4> sharedMemoryA,
        SPtr<uint4> sharedMemoryB,
        Storage weightStorage,
        Storage.Address weightAddress)
            where T : __BuiltinFloatingPointType
            where T.Differential == T
            where Storage : IStorage<T>
    {
        // lane id in a sub-group
        const uint landId = WaveGetLaneIndex();
        // get the 2-D coordinate of the tile in the vectorized tile
        const uint xInTile = landId % vectorizedTileWidthA;
        const uint yInTile = landId / vectorizedTileWidthA;

        // get the 2-D coordinate of the tile in original data type
        const uint tx = landId % T.COLUMN;
        const uint ty = landId / T.COLUMN;

        // pre-load the Matrix A into shared memory.
        // We load a column of matrix tile of Matrix A each time, because our iteration is over K dimension.
        // We require the Matrix A is stored as row major in the weight storage.
        // Also we load the tile into shared memory in row major.
        uint indexInVectorizedTile = yInTile * vectorizedTileWidthA + xInTile;
        uint indexInWeight = ty * ElementCountPerRowA + tx;
        for (uint i = 0; i < numLoadsEachThreadMatA; i++)
        {
            // Get the 1-D index in the weight matrix for current thread, note since this the first tile, so
            // there is no offset here. Otherwise, the offset will be the tile index.
            // TODO: Bounds check.
            let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
            uint4 value = weightStorage.readUint4(offsetAddress);

            sharedMemoryA[indexInVectorizedTile] = value;
            indexInVectorizedTile += offsetPerThreadLoadMatA;

            // Given the index in tile, we need to get the index in weight matrix
            // newTx and newTy are the x and y coordinates in tile but but measured by element T.
            // convert offset into 2-D coordinate in tile. So we can use this dx and dy to get
            // the offset in weight matrix. Because on 2-D, [dx, dy] within the tile is same as
            // [dx, dy] in weight matrix. The only difference is that we need to measure the dx
            // in length of T. So we need to multiply the vectorizedColNum to get the actual dx.
            const uint dx = (dxPerThreadLoadMatA % vectorizedTileWidthA) * elementCountPerVector;
            const uint dy = dyPerThreadLoadMatA * vectorizedTileWidthA;
            indexInWeight = indexInWeight + dy * ElementCountPerRowA + dx;
        }

        // pre-load the Matrix B into shared memory.
        // Since we use subgroup size as N, it's natural that the Matrix B is stored as column major,
        // as each column can only be collected by one thread in the subgroup.
        // We load a row of matrix tile of Matrix B each time, so the iteration is vectorized Tile Height.
        // TODO: If less than subgroup size threads are launched, then there are no other threads in a warp can fill the shared
        // memory for Matrix B. We need to handle this case as well. The same problem exists for Matrix A as well.

        indexInVectorizedTile = xInTile * vectorizedTileHeightB;
        for (uint i = 0; i < vectorizedTileHeightB; i++)
        {
            // TODO: Bounds check.
            uint4 value = readUint4(inputVector, i);
            sharedMemoryB[indexInVectorizedTile + i] = value;
        }

        // fetch first tile of Matrix A and Matrix B into shared memory
        // Iterate over the K dimension, T.COLUMN is column of Matrix A.
        uint doubleBufferSwitch = 0;
        for (int k = 0; k < K; k += T.COLUMN)
        {
            // warp sync to ensure all threads in the warp have loaded the data.
            AllMemoryBarrierWithWaveSync();

            // switch to another buffer
            doubleBufferSwitch = doubleBufferSwitch ^ 1; // flip between 0 and 1
            uint sharedMemOffsetMatA = doubleBufferSwitch * sharedMemSizeInBytesA;
            uint sharedMemOffsetMatB = doubleBufferSwitch * sharedMemSizeInBytesB;
            SPtr<uint4> shMemPtrMatA = sharedMemoryA + sharedMemOffsetMatA / sizeof(uint4);
            SPtr<uint4> shMemPtrMatB = sharedMemoryB + sharedMemOffsetMatB / sizeof(uint4);

            // TODO: Load the another buffer and do the math so we can hide the latency of the load.

            uint indexInVectorizedTile = yInTile * vectorizedTileWidthA + xInTile;

            // k is the x-coordinate of tile id, so (xInTile + k * vectorizedTileWidthA) is the x-coordinate in weight matrix,
            // but measured by vector uint4, so multiply elementCountPerVector to get the actual x-coordinate measured by element T.
            uint indexInWeight = yInTile * ElementCountPerRowA + (xInTile + k * vectorizedTileWidthA) * elementCountPerVector;
            uint offset = 0;
            for (uint i = 0; i < numLoadsEachThreadMatA; i++)
            {
                // Get the 1-D index in the weight matrix for current thread, note since this the first tile, so
                // there is no offset here. Otherwise, the offset will be the tile index.
                // TODO: Bounds check.
                let offsetAddress = Storage.getOffset(weightAddress, indexInWeight);
                uint4 value = weightStorage.readUint4(offsetAddress);

                shMemPtrMatA[indexInVectorizedTile] = value;
                indexInVectorizedTile += offsetPerThreadLoadMatA;

                // Given the index in tile, we need to get the index in weight matrix
                // newTx and newTy are the x and y coordinates in tile but but measured by element T.
                // convert offset into 2-D coordinate in tile. So we can use this dx and dy to get
                // the offset in weight matrix. Because on 2-D, [dx, dy] within the tile is same as
                // [dx, dy] in weight matrix. The only difference is that we need to measure the dx
                // in length of T. So we need to multiply the vectorizedColNum to get the actual dx.
                const uint dx = (dxPerThreadLoadMatA % vectorizedTileWidthA) * elementCountPerVector;
                const uint dy = dyPerThreadLoadMatA * vectorizedTileWidthA;
                indexInWeight = indexInWeight + dy * ElementCountPerRowA + dx;
            }

            indexInVectorizedTile = xInTile * vectorizedTileHeightB;
            for (uint i = 0; i < vectorizedTileHeightB; i++)
            {
                // TODO: Bounds check.
                uint4 value = readUint4(inputVector, i);
                sharedMemoryB[indexInVectorizedTile + i] = value;
            }

            // Math loop
            // switch to previous buffer to perform the math.
            doubleBufferSwitch = doubleBufferSwitch ^ 1; // flip between 0 and 1
            sharedMemOffsetMatA = doubleBufferSwitch * sharedMemSizeInBytesA;
            sharedMemOffsetMatB = doubleBufferSwitch * sharedMemSizeInBytesB;
            shMemPtrMatA = sharedMemoryA + sharedMemOffsetMatA / sizeof(uint4);
            shMemPtrMatB = sharedMemoryB + sharedMemOffsetMatB / sizeof(uint4);

            // TODO: rounding
            const uint numRows = (M + T.ROW - 1) / T.ROW;
            const uint numColumns = (N + T.COLUMN - 1) / T.COLUMN;
            MatA matA[numRows];
            for (int i = 0; i < numRows; i ++)
            {
                int stride = vectorizedTileWidthA * sizeof(uint4);
                matA[i].Load<linalg.CoopMatMatrixLayout.RowMajor, uint4>(shMemPtrMatA + i * T.ROW * stride, stride);
            }

            for (int j = 0; j < numColumns; j ++)
            {
                for (uint i = 0; i < numRows; i ++)
                {
                }
                // TODO: Math loop.
            }

        }
    }
}

// Cooperative matrix is only supported by CUDA and SPIR-V
[require(cuda_spirv)]
public struct AccelerateVectorCoopMat<T, int N> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = AccelerateVectorCoopMat<T.Differential, N>;

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
