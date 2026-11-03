//TEST(compute):COMPARE_COMPUTE:-dx12 -output-using-type -Xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE:-dx12 -output-using-type -Xslang -matrix-layout-column-major
//TEST:SIMPLE(filecheck=ASM-ROW):-target dxil-asm -entry computeMain -stage compute -matrix-layout-row-major -O3
//TEST:SIMPLE(filecheck=ASM-COLUMN):-target dxil-asm -entry computeMain -stage compute -matrix-layout-column-major -O3

// Exercise both out and inout aggregate copies with a matrix layout different from the target
// default. The nested array also checks that the call's complete value type is considered.
// Unrolling exposes constant component indices, which trigger DXC's faulty parameter copying.
// The assembly checks use optimization so the resulting logical elements appear as constants.

// ASM-ROW: @dx.op.bufferStore.i32({{.*}} i32 7, i32 0, i32 7,
// ASM-COLUMN: @dx.op.bufferStore.i32({{.*}} i32 1, i32 0, i32 1,

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

typealias RowMatrix = matrix<float, 2, 3, MatrixLayoutMode.RowMajor>;
typealias ColumnMatrix = matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor>;

struct MixedMatrices
{
    RowMatrix row;
    ColumnMatrix column;
}

struct MatrixArray
{
    MixedMatrices values[1];
}

void fillMatrices(out MixedMatrices value)
{
    [ForceUnroll] for (int row = 0; row < 2; ++row)
    [ForceUnroll] for (int column = 0; column < 3; ++column)
    {
        value.row[row][column] = row * 3 + column;
        value.column[row][column] = 6 + row * 3 + column;
    }
}

void incrementMatrices(inout MatrixArray value)
{
    [ForceUnroll] for (int row = 0; row < 2; ++row)
    [ForceUnroll] for (int column = 0; column < 3; ++column)
    {
        value.values[0].row[row][column] += 10 * (row + 1) + column;
        value.values[0].column[row][column] += 20 * (row + 1) + column;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MatrixArray matrices;
    fillMatrices(matrices.values[0]);
    [ForceUnroll] for (int row = 0; row < 2; ++row)
    [ForceUnroll] for (int column = 0; column < 3; ++column)
    {
        int index = row * 3 + column;
        outputBuffer[index] = int(matrices.values[0].row[row][column]);
        outputBuffer[6 + index] = int(matrices.values[0].column[row][column]);
    }

    incrementMatrices(matrices);
    [ForceUnroll] for (int row = 0; row < 2; ++row)
    [ForceUnroll] for (int column = 0; column < 3; ++column)
    {
        int index = row * 3 + column;
        outputBuffer[12 + index] = int(matrices.values[0].row[row][column]);
        outputBuffer[18 + index] = int(matrices.values[0].column[row][column]);
    }
}
