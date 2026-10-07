// An explicit `row_major` / `column_major` written before a modern-syntax declaration
// (`row_major var values: float2x3[2];`) applies to the matrix element of an array type, the same
// as the declarator `row_major float2x3 values[2];` and the type-side `var values: row_major
// float2x3[2];` (shader-slang/slang#13390). This covers struct fields, a 2-D array, the
// `column_major` mirror, `cbuffer` members, `uniform` globals and modern parameters.

//TEST:REFLECTION(filecheck=COL):-target hlsl -profile sm_6_0 -no-codegen -matrix-layout-column-major
//TEST:REFLECTION(filecheck=ROW):-target hlsl -profile sm_6_0 -no-codegen -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry main -stage compute -matrix-layout-column-major

// A row-major `float2x3[2]` is 60 bytes with a 32-byte element stride, a column-major one 88
// bytes with a 48-byte stride (see matrix-layout-array-param.slang). Each array is followed by a
// scalar that packs right after it. Reflection lists globals in declaration order, then the
// entry point's parameters.

// COL: "name": "cbValues"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "cbTrailing"
// COL: "offset": 60
// ROW: "name": "cbValues"
// ROW: "offset": 0, "size": 60, "elementStride": 32
// ROW: "name": "cbTrailing"
// ROW: "offset": 60
cbuffer Constants
{
    row_major var cbValues: float2x3[2];
    var cbTrailing: float;
}

// COL: "name": "globalValues"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "globalTrailing"
// COL: "offset": 60
uniform row_major var globalValues: float2x3[2];
uniform var globalTrailing: float;

struct RowMajorFields
{
    row_major var values: float2x3[2];
    var trailingValue: float;
}

// Each inner array `row_major float2x3[2]` is 60 bytes with a 64-byte stride, so the whole array
// is 64 + 60 = 124 bytes.
struct RowMajorFields2D
{
    row_major var values: float2x3[2][2];
    var trailingValue: float;
}

struct ColumnMajorFields
{
    column_major var values: float2x3[2];
    var trailingValue: float;
}

// COL: "name": "rowMajorFields"
// COL: "name": "values"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "trailingValue"
// COL: "offset": 60
// ROW: "name": "rowMajorFields"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 60, "elementStride": 32
ConstantBuffer<RowMajorFields> rowMajorFields;

// COL: "name": "rowMajorFields2D"
// COL: "name": "values"
// COL: "offset": 0, "size": 124, "elementStride": 64
// COL: "name": "trailingValue"
// COL: "offset": 124
ConstantBuffer<RowMajorFields2D> rowMajorFields2D;

// ROW: "name": "columnMajorFields"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 88, "elementStride": 48
// ROW: "name": "trailingValue"
// ROW: "offset": 88
// COL: "name": "columnMajorFields"
// COL: "name": "values"
// COL: "offset": 0, "size": 88, "elementStride": 48
ConstantBuffer<ColumnMajorFields> columnMajorFields;

RWStructuredBuffer<float> outputBuffer;

// COL: "name": "paramValues"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "paramTrailing"
// COL: "offset": 60
// ROW: "name": "paramValues"
// ROW: "offset": 0, "size": 60, "elementStride": 32
[shader("compute")]
[numthreads(1, 1, 1)]
func main(uniform row_major paramValues: float2x3[2], uniform paramTrailing: float) -> void
{
    outputBuffer[0] = cbValues[0][0][0] + cbTrailing + globalValues[0][0][0] + globalTrailing +
                      rowMajorFields.values[0][0][0] + rowMajorFields.trailingValue +
                      rowMajorFields2D.values[1][1][0][0] + columnMajorFields.values[0][0][0] +
                      paramValues[0][0][0] + paramTrailing;
}

// The layouts reach the SPIR-V decorations: Slang `row_major` maps to SPIR-V `ColMajor` (the
// convention is inverted). A row-major array has a 32-byte stride, the 2-D array a 64-byte outer
// stride, and the column-major mirror a 48-byte stride and `RowMajor`; each trailing scalar
// follows its array.
// SPV-DAG: OpDecorate %_arr_mat2v3float_int_2 ArrayStride 32
// SPV-DAG: OpMemberDecorate %_Array_std140_matrix_float_2_3_2 0 ColMajor
// SPV-DAG: OpMemberDecorate %SLANG_ParameterGroup_Constants_std140 1 Offset 64
// SPV-DAG: OpMemberDecorate %GlobalParams_std140 1 Offset 64
// SPV-DAG: OpMemberDecorate %RowMajorFields_std140 1 Offset 64
// SPV-DAG: OpDecorate %_arr__Array_std140_matrix_float_2_3_2_int_2 ArrayStride 64
// SPV-DAG: OpMemberDecorate %RowMajorFields2D_std140 1 Offset 128
// SPV-DAG: OpDecorate %_arr_mat2v3float_int_2_0 ArrayStride 48
// SPV-DAG: OpMemberDecorate %_Array_std140_matrix_float_2_3_2_0 0 RowMajor
// SPV-DAG: OpMemberDecorate %ColumnMajorFields_std140 1 Offset 96
// SPV-DAG: OpMemberDecorate %_Array_std430_matrix_float_2_3_2 0 ColMajor
// SPV-DAG: OpMemberDecorate %EntryPointParams_std430 1 Offset 64
