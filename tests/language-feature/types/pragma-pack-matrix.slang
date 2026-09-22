//TEST:SIMPLE: -target hlsl -profile cs_6_0 -entry main -no-codegen -matrix-layout-column-major
//TEST:REFLECTION(filecheck=REFLECT): -target hlsl -profile cs_6_0 -entry main -no-codegen -matrix-layout-column-major

module pragma_pack_matrix_test;

typedef matrix<float, 2, 3, MatrixLayoutMode.RowMajor> RowMatrix;
typedef matrix<float, 2, 3, MatrixLayoutMode.ColumnMajor> ColumnMatrix;
typedef matrix<float, 4, 4, MatrixLayoutMode.RowMajor> RowSquare;

struct MatrixBox<T>
{
    T value;
}

static const int kVectorSize = 2;

#define MATRIX_TYPE float2x3

#pragma pack_matrix(row_major)
__include "pragma-pack-matrix-semantic-include.slang";

static float2x3 afterSemanticInclude;
typedef float2x3 SavedRowMatrix;
static MATRIX_TYPE macroRowMatrix;

// The semantic include starts from the configured column-major default rather than inheriting
// this file's row-major pragma.
// REFLECT: "name": "semanticIncludeInitialTail"
// REFLECT: "offset": 40

float2x3 passRowMatrix(float2x3 value)
{
    return value;
}

// Function bodies are parsed later. This body must retain the row-major state active here even
// after preprocessing reaches the column-major directive below it.
void checkRowMajorBody()
{
    float2x3 declared;
    let constructed = float2x3(0);
    let castValue = (float2x3)0;
    let genericConstructed = matrix<float, 2, 3>(0);
    let parenthesizedConstructed = (matrix<float, 2, 3>)(0);
    matrix bare;
    column_major float2x3 explicitColumn;
    MatrixBox<float2x3[2]> arrayBox;
    let throughSignature = passRowMatrix(constructed);

    static_assert(declared is RowMatrix, "declaration uses source default");
    static_assert(constructed is RowMatrix, "constructor uses source default");
    static_assert(castValue is RowMatrix, "cast uses source default");
    static_assert(genericConstructed is RowMatrix, "generic matrix uses source default");
    static_assert(
        parenthesizedConstructed is RowMatrix,
        "parenthesized matrix uses source default");
    static_assert(bare is RowSquare, "bare matrix resolves its default arguments");
    static_assert(explicitColumn is ColumnMatrix, "explicit layout overrides source default");
    static_assert(arrayBox.value is RowMatrix[2], "array element uses source default");
    static_assert(throughSignature is RowMatrix, "function signature uses source default");

#pragma pack_matrix(column_major)
    float2x3 switched;
    static_assert(switched is ColumnMatrix, "directive changes deferred body state");
}

static MATRIX_TYPE macroColumnMatrix;

void checkSymbolicLayout<let L : MatrixLayoutMode, Expected>()
{
    static_assert(matrix<float, 2, 3, L> is Expected, "explicit symbolic layout is preserved");
}

[numthreads(1, 1, 1)]
void main()
{
    checkRowMajorBody();

    float2x3 current;
    row_major float2x3 explicitRow;
    vector<int, kVectorSize> values = int2(1, 2);

    static_assert(current is ColumnMatrix, "later directive changes source default");
    static_assert(explicitRow is RowMatrix, "explicit layout overrides later source default");
    static_assert(SavedRowMatrix is RowMatrix, "ordinary typedef retains definition layout");
    static_assert(semanticIncludeRow is RowMatrix, "semantic include handles its own directive");
    static_assert(
        afterSemanticInclude is RowMatrix,
        "semantic include does not change including source default");
    static_assert(macroRowMatrix is RowMatrix, "macro type uses row-major invocation state");
    static_assert(
        macroColumnMatrix is ColumnMatrix,
        "macro type uses column-major invocation state");
    static_assert(values is int2, "non-matrix generic value is unaffected");
    checkSymbolicLayout<MatrixLayoutMode.RowMajor, RowMatrix>();
}
