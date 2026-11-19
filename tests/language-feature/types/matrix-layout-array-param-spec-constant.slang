// A layout-qualified array parameter whose length is a specialization constant accepts only an
// argument of the same layout: the array layout conversion applies to literal lengths only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

[SpecializationConstant]
const int SK = 2;

struct RowMajorMatrices
{
    row_major float2x3 values[SK];
}

RWStructuredBuffer<RowMajorMatrices> rowMajorStorage;
RWStructuredBuffer<float> output;

float byValue(row_major float2x3 m[SK]) { return m[1][0][1]; }

[numthreads(1, 1, 1)]
void main()
{
    float2x3 plain[SK];
    output[0] = byValue(plain);
//CHECK:                ^^^^^ E30019
//CHECK:                ^^^^^ expected an expression of type
    output[1] = byValue(rowMajorStorage[0].values);
}
