// Regression for shader-slang/slang#12990: the fix reclassifies both `RowMajorLayoutModifier`
// and `ColumnMajorLayoutModifier`, so exercise the column-major direction through the fixed path
// (the array test covers row-major). An explicit `column_major` on an array-of-matrix member is
// honored even when the command-line default is row-major.

//TEST:REFLECTION(filecheck=REFLECT):-profile cs_6_0 -target hlsl -no-codegen -matrix-layout-row-major

cbuffer Params
{
    column_major float2x3 values[2];
    float trailingValue;
}

// A `column_major float2x3` sizes each column on a 16-byte boundary (element stride 48, element
// data 40), so the array size is (2-1)*48 + 40 = 88 and the trailing scalar lands at offset 88 —
// distinct from the row-major numbers (stride 32, offset 60), which is what the dropped modifier
// silently produced under the row-major default.
// REFLECT: "name": "values"
// REFLECT: "offset": 0, "size": 88, "elementStride": 48
// REFLECT: "name": "trailingValue"
// REFLECT: "offset": 88

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}
