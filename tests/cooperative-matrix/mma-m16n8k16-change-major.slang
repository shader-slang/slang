//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// MMA m16n8k16: test ChangeMajor (in-register transpose) on Matrix A.
//
// Test 1: C = A * B (no transpose)
//   A[i][j] = i (row index), B = all 1.0
//   C[i][j] = sum(A[i][k] * 1.0, k=0..15) = i * 16
//
// Test 2: C2 = A^T * B (A transposed via ChangeMajor)
//   A^T[i][j] = j (original col becomes row after transpose)
//   C2[i][j] = sum(A^T[i][k] * 1.0, k=0..15) = sum(k, k=0..15) = 120
//   (every row of A^T has values 0..15, so sum = 0+1+...+15 = 120)
//
// Result buffer: 2 floats
//   [0] = 1.0 if Test 1 passed (C = A*B without transpose)
//   [1] = 1.0 if Test 2 passed (C2 = A^T*B with ChangeMajor)

// CHECK: type: float
// CHECK-COUNT-2: 1.0

//TEST_INPUT:ubuffer(stride=2, count=256),name=inputA
RWStructuredBuffer<half> inputA;

//TEST_INPUT:ubuffer(stride=2, count=128),name=inputB
RWStructuredBuffer<half> inputB;

//TEST_INPUT:ubuffer(stride=4, count=128),name=outputC1
RWStructuredBuffer<float> outputC1;

//TEST_INPUT:ubuffer(stride=4, count=128),name=outputC2
RWStructuredBuffer<float> outputC2;

//TEST_INPUT:ubuffer(stride=4, count=2):out,name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

using namespace linalg;

void verifyRegion(
    RWStructuredBuffer<float> output,
    uint offset,
    uint rows,
    uint cols,
    uint stride,
    float expectedFn_row,
    float expectedFn_scale,
    bool isConstant,
    float constantVal,
    RWStructuredBuffer<float> result,
    uint resultIdx)
{
    float pass = 1.0;
    for (uint row = 0; row < rows; row++)
    {
        for (uint col = 0; col < cols; col++)
        {
            float expected;
            if (isConstant)
                expected = constantVal;
            else
                expected = float(row) * expectedFn_scale;

            uint addr = offset + row * stride + col;
            if (output[addr] != expected)
                pass = 0.0;
        }
    }
    result[resultIdx] = pass;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // Fill A row-major: A[row][col] = half(row)
    for (uint i = tid; i < 256; i += 32)
        inputA[i] = half(float(i / 16));

    // Fill B row-major: all 1.0
    for (uint i = tid; i < 128; i += 32)
        inputB[i] = half(1.0);

    GroupMemoryBarrierWithGroupSync();

    typealias MatA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
    typealias MatB = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>;
    typealias MatC = CoopMat<float, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixAccumulator>;

    let zero = MatC(0.0);

    // Load A and B
    var matA = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.RowMajor>(inputA, 0, 16);
    let matB = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.RowMajor>(inputB, 0, 8);

    // Test 1: C = A * B (no transpose)
    // Expected: C[i][j] = i * 16
    let c1 = coopMatMulAdd<float, false>(matA, matB, zero);
    c1.Store<CoopMatMatrixLayout.RowMajor>(outputC1, 0, 8);

    // Test 2: ChangeMajor on A, then C2 = A^T * B
    // A^T[i][j] = j, so C2[i][j] = sum(j, j=0..15) = 120
    matA.ChangeMajor();
    let c2 = coopMatMulAdd<float, false>(matA, matB, zero);
    c2.Store<CoopMatMatrixLayout.RowMajor>(outputC2, 0, 8);

    GroupMemoryBarrierWithGroupSync();

    // Verify both results (only thread 0)
    if (tid == 0)
    {
        // Test 1: C[row][col] = row * 16
        float pass1 = 1.0;
        for (uint row = 0; row < 16; row++)
        {
            for (uint col = 0; col < 8; col++)
            {
                float expected = float(row) * 16.0;
                if (outputC1[row * 8 + col] != expected)
                    pass1 = 0.0;
            }
        }
        resultBuffer[0] = pass1;

        // Test 2: C2[row][col] = 120.0 for all elements
        float pass2 = 1.0;
        for (uint row = 0; row < 16; row++)
        {
            for (uint col = 0; col < 8; col++)
            {
                float expected = 120.0;
                if (outputC2[row * 8 + col] != expected)
                    pass2 = 0.0;
            }
        }
        resultBuffer[1] = pass2;
    }
}
