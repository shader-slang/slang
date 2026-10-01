// Regression for shader-slang/slang#13376: each way of reaching a plain array of matrices from an
// array with an explicit matrix layout compiles, under either default layout. The value test
// `matrix-layout-array-coerce.slang` checks that the converted values are correct.

//TEST:SIMPLE(filecheck=SPV):-target spirv -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry main -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -matrix-layout-column-major

struct S
{
    row_major float4x4 m[2];
}

cbuffer C
{
    row_major float4x4 rowMajor[2];
    column_major float4x4 columnMajor[2];
    S s;
    row_major float4x4 single;
}

StructuredBuffer<S> sb;
RWStructuredBuffer<float> output;

float use(float4x4 x[2]) { return x[1][2][3]; }
float useOne(float4x4 x) { return x[2][3]; }

[numthreads(1, 1, 1)]
void main()
{
    // A cbuffer array passed to a plain array parameter.
    output[0] = use(rowMajor);

    float4x4 local[2] = rowMajor;
    output[1] = local[1][2][3];
    float4x4 local2[2] = columnMajor;
    output[2] = local2[1][2][3];

    // A struct field array assigned to a plain local array.
    float4x4 local3[2] = s.m;
    output[3] = local3[1][2][3];

    // A `StructuredBuffer` element's field passed to a plain array parameter.
    output[4] = use(sb[0].m);

    // Controls: a single matrix, an indexed element, a whole-struct copy, and a target with the
    // same layout.
    output[5] = useOne(single);
    output[6] = useOne(rowMajor[1]);
    S t = s;
    output[7] = t.m[1][2][3];
    row_major float4x4 same[2] = rowMajor;
    output[8] = same[1][2][3];
}

// The explicit layouts stay on the buffer arrays (Slang `row_major` is SPIR-V `ColMajor`), and the
// converted value is a separate plain array that is built from the loaded elements.
// SPV-DAG: OpMemberDecorate %{{.*}} 0 ColMajor
// SPV-DAG: OpMemberDecorate %{{.*}} 0 RowMajor
// HLSL: float use_0(float4x4  x_0[int(2)])
// HLSL: float4x4  [[ARG:_S[0-9]+]][int(2)] = {
// HLSL: use_0([[ARG]])
