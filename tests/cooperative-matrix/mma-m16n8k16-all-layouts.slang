//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// MMA m16n8k16: test all 8 layout combinations of A, B, C
// A: 16x16 (half), A[row][col] = row
// B: 16x8  (half), all 1.0
// D[i][j] = sum(i * 1.0, k=0..15) = i * 16
//
// 4 A/B combos x 2 C store layouts = 8 total combinations.
// Result buffer: 8 floats, each 1.0 if that combination passed.

// CHECK: type: float
// CHECK-COUNT-8: 1.0

//TEST_INPUT:ubuffer(stride=2, count=256),name=inputA_row
RWStructuredBuffer<half> inputA_row;

//TEST_INPUT:ubuffer(stride=2, count=256),name=inputA_col
RWStructuredBuffer<half> inputA_col;

//TEST_INPUT:ubuffer(stride=2, count=128),name=inputB_row
RWStructuredBuffer<half> inputB_row;

//TEST_INPUT:ubuffer(stride=2, count=128),name=inputB_col
RWStructuredBuffer<half> inputB_col;

//TEST_INPUT:ubuffer(stride=4, count=512),name=outputRow
RWStructuredBuffer<float> outputRow;

//TEST_INPUT:ubuffer(stride=4, count=512),name=outputCol
RWStructuredBuffer<float> outputCol;

//TEST_INPUT:ubuffer(stride=4, count=8):out,name=resultBuffer
RWStructuredBuffer<float> resultBuffer;

using namespace linalg;

typealias MatA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias MatB = CoopMat<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB>;
typealias MatC = CoopMat<float, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixAccumulator>;

void verifyRegion(
    RWStructuredBuffer<float> output,
    uint offset,
    bool isColumnMajor,
    RWStructuredBuffer<float> result,
    uint resultIdx)
{
    float pass = 1.0;
    for (uint row = 0; row < 16; row++)
    {
        for (uint col = 0; col < 8; col++)
        {
            float expected = float(row) * 16.0;
            uint addr;
            if (isColumnMajor)
                addr = offset + col * 16 + row;
            else
                addr = offset + row * 8 + col;
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
    // Fill A row-major: buffer[row * 16 + col] = half(row)
    for (uint i = tid; i < 256; i += 32)
        inputA_row[i] = half(float(i / 16));

    // Fill A column-major: buffer[col * 16 + row] = half(row)
    for (uint i = tid; i < 256; i += 32)
        inputA_col[i] = half(float(i % 16));

    // Fill B row-major and column-major: all 1.0
    for (uint i = tid; i < 128; i += 32)
        inputB_row[i] = half(1.0);
    for (uint i = tid; i < 128; i += 32)
        inputB_col[i] = half(1.0);

    GroupMemoryBarrierWithGroupSync();

    let zero = MatC(0.0);

    // --- Combo 0: A_row * B_row ---
    {
        let a = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.RowMajor>(inputA_row, 0, 16);
        let b = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.RowMajor>(inputB_row, 0, 8);
        let d = coopMatMulAdd<float, false>(a, b, zero);
        d.Store<CoopMatMatrixLayout.RowMajor>(outputRow, 0, 8);
        d.Store<CoopMatMatrixLayout.ColumnMajor>(outputCol, 0, 16);
    }

    // --- Combo 1: A_row * B_col ---
    {
        let a = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.RowMajor>(inputA_row, 0, 16);
        let b = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.ColumnMajor>(inputB_col, 0, 16);
        let d = coopMatMulAdd<float, false>(a, b, zero);
        d.Store<CoopMatMatrixLayout.RowMajor>(outputRow, 128, 8);
        d.Store<CoopMatMatrixLayout.ColumnMajor>(outputCol, 128, 16);
    }

    // --- Combo 2: A_col * B_row ---
    {
        let a = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.ColumnMajor>(inputA_col, 0, 16);
        let b = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.RowMajor>(inputB_row, 0, 8);
        let d = coopMatMulAdd<float, false>(a, b, zero);
        d.Store<CoopMatMatrixLayout.RowMajor>(outputRow, 256, 8);
        d.Store<CoopMatMatrixLayout.ColumnMajor>(outputCol, 256, 16);
    }

    // --- Combo 3: A_col * B_col ---
    {
        let a = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA, CoopMatMatrixLayout.ColumnMajor>(inputA_col, 0, 16);
        let b = coopMatLoad<half, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.ColumnMajor>(inputB_col, 0, 16);
        let d = coopMatMulAdd<float, false>(a, b, zero);
        d.Store<CoopMatMatrixLayout.RowMajor>(outputRow, 384, 8);
        d.Store<CoopMatMatrixLayout.ColumnMajor>(outputCol, 384, 16);
    }

    // --- Verify all 8 combinations (only thread 0) ---
    GroupMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        // 4 combos x row-major output
        verifyRegion(outputRow,   0, false, resultBuffer, 0);
        verifyRegion(outputRow, 128, false, resultBuffer, 1);
        verifyRegion(outputRow, 256, false, resultBuffer, 2);
        verifyRegion(outputRow, 384, false, resultBuffer, 3);
        // 4 combos x column-major output
        verifyRegion(outputCol,   0, true, resultBuffer, 4);
        verifyRegion(outputCol, 128, true, resultBuffer, 5);
        verifyRegion(outputCol, 256, true, resultBuffer, 6);
        verifyRegion(outputCol, 384, true, resultBuffer, 7);
    }
}
