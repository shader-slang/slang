implementing neural;

#include "common-def.slang"

// MMAHelperNew: Tiled MMA using m16n8k16 with no shared memory at all.
//
// MatA (weight): loaded directly from tiled global memory into CoopMat registers.
//   The weight matrix must be pre-tiled into 16×16 tiles in row-major tile order.
//   M and K must be multiples of 16.
//
// MatB (input): constructed from each thread's local input vector via FromLocalColumn.
//
// MatC (output): extracted to each thread's local output vector via ToLocalColumn.
//
// MMA shape: m16n8k16
//   MatA: 16×16 (half)
//   MatB: 16×8  (half)
//   MatC: 16×8  (T)
//
// For a 32-thread warp with N_tile=8: 4 MatB/MatC tiles cover all 32 threads.

[require(cuda)]
VISIBILITY_LEVEL struct MMAHelperNew<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const int M = OutputSize;
    static const int N = SubgroupSize;
    static const int K = InputSize;

    // m16n8k16 shape constants
    static const int ROW_A = 16;
    static const int COLUMN_A = 16;
    static const int ROW_B = 16;
    static const int COLUMN_B = 8;
    static const int ROW_C = 16;
    static const int COLUMN_C = 8;

    static const uint ElementCountPerVector = sizeof(uint4) / sizeof(half);

    // Tiling counts
    VISIBILITY_LEVEL static const int NCoopMatRow = (M + ROW_A - 1) / ROW_A;
    VISIBILITY_LEVEL static const int NCoopMatColumn = (N + COLUMN_B - 1) / COLUMN_B;
    VISIBILITY_LEVEL static const int NCoopMat = NCoopMatRow * NCoopMatColumn;

    VISIBILITY_LEVEL static const int Uint4AlignedK = ((K + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    static const linalg.CoopMatMatrixLayout RowMajor = linalg.CoopMatMatrixLayout.RowMajor;
    static const linalg.CoopMatMatrixLayout ColumnMajor = linalg.CoopMatMatrixLayout.ColumnMajor;

    typealias MatA = linalg.CoopMat<half, MemoryScope.Subgroup, ROW_A, COLUMN_A, linalg.CoopMatMatrixUse.MatrixA>;
    typealias MatB = linalg.CoopMat<half, MemoryScope.Subgroup, ROW_B, COLUMN_B, linalg.CoopMatMatrixUse.MatrixB>;
    typealias MatC = linalg.CoopMat<T,    MemoryScope.Subgroup, ROW_C, COLUMN_C, linalg.CoopMatMatrixUse.MatrixAccumulator>;

    // =========================================================================
    // Tiled-layout addressing for MatA
    // =========================================================================

    static const int NTileRows = (M + ROW_A - 1) / ROW_A;
    static const int NTileCols = (K + COLUMN_A - 1) / COLUMN_A;
    static const int TileSizeInElements = ROW_A * COLUMN_A;

    [ForceInline]
    VISIBILITY_LEVEL static int tileElementOffset(int tileRow, int tileCol)
    {
        return (tileRow * NTileCols + tileCol) * TileSizeInElements;
    }

    // Load a single CoopMat A tile directly from tiled global memory.
    // tileRowIdx indexes M dimension, tileColIdx indexes K dimension.
    [ForceInline]
    [require(cuda)]
    VISIBILITY_LEVEL static MatA loadTileDirect<U, Address>(
        int tileRowIdx,
        int tileColIdx,
        Address weightAddress)
            where U : __BuiltinFloatingPointType
            where U.Differential == U
            where Address : IPointerLikeAddress<U>
    {
        int offset = tileElementOffset(tileRowIdx, tileColIdx);
        let tileAddr = weightAddress.getOffset(offset);
        return bit_cast<MatA>(loadCoopMat<U, Address, ROW_A, COLUMN_A, linalg.CoopMatMatrixUse.MatrixA, RowMajor>(
            tileAddr, 0u, uint(COLUMN_A)));
    }

    // =========================================================================
    // Tiled MMA: zero shared memory
    // =========================================================================

    // Tiled MMA: C = A * B
    //   A: loaded directly from tiled global memory (no shared memory)
    //   B: constructed from thread-local columns via FromLocalColumn
    //   C: extracted to thread-local columns via ToLocalColumn
    //
    // Each thread holds an input vector of length K and produces an output
    // vector of length M. The warp collectively performs the matrix-vector
    // multiply: output[i] = sum_k(Weight[i][k] * input[k]) for each thread.
    [ForceInline]
    [require(cuda)]
    VISIBILITY_LEVEL static OutArrayType mma<U, Address, InArrayType, OutArrayType, V, bool Bias = false>(
        InArrayType inputVector,
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
        MatC matC[NCoopMat];
        for (int i = 0; i < NCoopMat; i++)
            matC[i].fill(T(0.0f));

        OutArrayType outputVector;
        if (Bias)
        {
            for (int i = 0; i < M; i++)
            {
                U bias = biasAddress.value[uint(i)];
                outputVector[i] = __realCast<V>(bias);
            }
        }

        for (int k = 0; k < Uint4AlignedK; k += COLUMN_A)
        {
            int tileColIdx = k / COLUMN_A;

            MatA matA[NCoopMatRow];
            for (uint i = 0; i < NCoopMatRow; i++)
                matA[i] = loadTileDirect<U, Address>(int(i), tileColIdx, weightAddress);

            for (uint j = 0; j < NCoopMatColumn; j++)
            {
                half localColB[ROW_B];
                for (uint r = 0; r < ROW_B; r++)
                {
                    int srcIdx = k + r;
                    localColB[r] = (srcIdx < K) ? __realCast<half>(inputVector[srcIdx]) : half(0.0);
                }

                let matB = MatB.FromLocalColumn(localColB, int(j));

                for (uint i = 0; i < NCoopMatRow; i++)
                {
                    int index = i * NCoopMatColumn + int(j);
                    matC[index] = linalg.coopMatMulAdd<T, false, half, half, T,
                                            MemoryScope.Subgroup, ROW_A, COLUMN_A, COLUMN_B
                                            >(matA[i], matB, matC[index]);
                }
            }
        }

        for (int i = 0; i < NCoopMatRow; i++)
        {
            for (int j = 0; j < NCoopMatColumn; j++)
            {
                int index = i * NCoopMatColumn + j;

                if (sizeof(T) == 4)
                {
                    T localColC[ROW_C];
                    matC[index].ToLocalColumn(localColC, j);

                    int rowBase = i * ROW_C;
                    for (int r = 0; r < ROW_C; r++)
                    {
                        int outIdx = rowBase + r;
                        if (outIdx < M)
                        {
                            if (Bias)
                                outputVector[outIdx] = outputVector[outIdx] + __realCast<V>(localColC[r]);
                            else
                                outputVector[outIdx] = __realCast<V>(localColC[r]);
                        }
                    }
                }
                else
                {
                    T localColC[ROW_C];
                    matC[index].ToLocalColumn(localColC, j);

                    int rowBase = i * ROW_C;
                    for (int r = 0; r < ROW_C; r++)
                    {
                        int outIdx = rowBase + r;
                        if (outIdx < M)
                        {
                            if (Bias)
                                outputVector[outIdx] = outputVector[outIdx] + __realCast<V>(localColC[r]);
                            else
                                outputVector[outIdx] = __realCast<V>(localColC[r]);
                        }
                    }
                }
            }
        }

        return outputVector;
    }
}
