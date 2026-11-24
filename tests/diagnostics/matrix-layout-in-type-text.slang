// A matrix type prints its layout, so types that differ only in layout read differently.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):-target spirv -stage compute -entry computeMain

struct Box<T>
{
    T v;
}

void takeGeneric<let L : MatrixLayoutMode>(matrix<float, 3, 3, L> m)
{
    int x = m;
//CHK:      ^ type mismatch in expression
//CHK:      ^ expected an expression of type 'int', got 'matrix<float,3,3,L>'
}

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    Box<row_major float3x3> a;
    a.v = float3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    Box<float3x3> b = a;
//CHK:                ^ type mismatch in expression
//CHK:                ^ expected an expression of type 'Box<matrix<float,3,3>>', got 'Box<matrix<float,3,3,MatrixLayoutMode.RowMajor>>'

    Box<column_major float3x3> c = a;
//CHK:                             ^ type mismatch in expression
//CHK:                             ^ expected an expression of type 'Box<matrix<float,3,3,MatrixLayoutMode.ColumnMajor>>', got 'Box<matrix<float,3,3,MatrixLayoutMode.RowMajor>>'

    output[0] = b.v[0][0] + c.v[0][0];
}

float pick(row_major float2x3 m) { return 1.0; }
//CHK:^^^^ candidate: func pick(matrix<float,2,3,MatrixLayoutMode.RowMajor>) -> float
float pick(column_major float2x3 m) { return 2.0; }
//CHK:^^^^ candidate: func pick(matrix<float,2,3,MatrixLayoutMode.ColumnMajor>) -> float

float callPick()
{
    float2x3 plain = {};
    return pick(plain);
//CHK:         ^ ambiguous call to 'pick' with arguments of type (matrix<float,2,3>)
}
