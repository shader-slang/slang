// Regression for shader-slang/slang#13375: an explicit `row_major` / `column_major` on an
// array-of-matrix parameter written in traditional (leading-modifier) syntax is honored, the same
// as on a cbuffer member or a modern-syntax `values: row_major float2x3[2]` parameter.

//TEST:REFLECTION(filecheck=COL):-target hlsl -profile sm_6_0 -no-codegen -matrix-layout-column-major
//TEST:REFLECTION(filecheck=ROW):-target hlsl -profile sm_6_0 -no-codegen -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry rowMajorArray -stage compute -matrix-layout-column-major

// Each entry point takes the array followed by a trailing scalar. A row-major `float2x3` has two
// 16-byte rows (element stride 32, data 28), so a two-element array is (2-1)*32 + 28 = 60 bytes;
// a column-major one has three 16-byte columns (stride 48, data 40), so it is 48 + 40 = 88 bytes.
// The trailing scalar packs right after the array either way. Entry points appear in declaration
// order; each contributes its parameters twice (entry-point scope, then parameter list).

RWStructuredBuffer<float> outputBuffer;

// The issue's repro: the explicit `row_major` wins over a column-major default.
// COL: "name": "rowMajorArray"
// COL: "name": "values"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "trailingValue"
// COL: "offset": 60
// ROW: "name": "rowMajorArray"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 60, "elementStride": 32
// ROW: "name": "trailingValue"
// ROW: "offset": 60
[shader("compute")]
[numthreads(1, 1, 1)]
void rowMajorArray(uniform row_major float2x3 values[2], uniform float trailingValue)
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}

// The mirror case: the explicit `column_major` wins over a row-major default.
// COL: "name": "columnMajorArray"
// COL: "name": "values"
// COL: "offset": 0, "size": 88, "elementStride": 48
// COL: "name": "trailingValue"
// COL: "offset": 88
// ROW: "name": "columnMajorArray"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 88, "elementStride": 48
// ROW: "name": "trailingValue"
// ROW: "offset": 88
[shader("compute")]
[numthreads(1, 1, 1)]
void columnMajorArray(uniform column_major float2x3 values[2], uniform float trailingValue)
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}

// Control: without a modifier the array follows the command-line default.
// COL: "name": "defaultArray"
// COL: "name": "values"
// COL: "offset": 0, "size": 88, "elementStride": 48
// ROW: "name": "defaultArray"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 60, "elementStride": 32
[shader("compute")]
[numthreads(1, 1, 1)]
void defaultArray(uniform float2x3 values[2], uniform float trailingValue)
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}

// A 2-D array keeps the layout at every level: the inner row is 60 bytes with a 64-byte stride,
// so the whole array is 64 + 60 = 124 bytes.
// COL: "name": "rowMajorArray2D"
// COL: "name": "values"
// COL: "offset": 0, "size": 124, "elementStride": 64
// COL: "name": "trailingValue"
// COL: "offset": 124
// ROW: "name": "rowMajorArray2D"
// ROW: "name": "values"
// ROW: "offset": 0, "size": 124, "elementStride": 64
[shader("compute")]
[numthreads(1, 1, 1)]
void rowMajorArray2D(uniform row_major float2x3 values[2][2], uniform float trailingValue)
{
    outputBuffer[0] = values[1][1][0][0] + trailingValue;
}

// The postfix spelling, with the brackets on the type rather than the name.
// COL: "name": "rowMajorPostfixArray"
// COL: "name": "values"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "trailingValue"
// COL: "offset": 60
[shader("compute")]
[numthreads(1, 1, 1)]
void rowMajorPostfixArray(uniform row_major float2x3[2] values, uniform float trailingValue)
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}

// A traditional-style parameter inside a modern `func` parameter list.
// COL: "name": "rowMajorArrayInModernFunc"
// COL: "name": "values"
// COL: "offset": 0, "size": 60, "elementStride": 32
// COL: "name": "trailingValue"
// COL: "offset": 60
[shader("compute")]
[numthreads(1, 1, 1)]
func rowMajorArrayInModernFunc(uniform row_major float2x3 values[2], uniform trailingValue: float)
    -> void
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}

// The layout reaches the SPIR-V element decorations: Slang `row_major` maps to SPIR-V `ColMajor`
// (the convention is inverted), with 16-byte matrix stride and the trailing scalar at offset 64.
// SPV: OpDecorate %_arr_mat2v3float_int_2 ArrayStride 32
// SPV: OpMemberDecorate %_Array_std430_matrix_float_2_3_2 0 ColMajor
// SPV: OpMemberDecorate %_Array_std430_matrix_float_2_3_2 0 MatrixStride 16
// SPV: OpMemberDecorate %EntryPointParams_std430 1 Offset 64
