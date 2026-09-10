// Regression for shader-slang/slang#12990: an explicit `row_major` on an array-of-matrix
// cbuffer member is honored.

//TEST:REFLECTION(filecheck=REFLECT):-profile cs_6_0 -target hlsl -no-codegen -matrix-layout-column-major

cbuffer Params
{
    row_major float2x3 values[2];
    float trailingValue;
}

// The array element keeps its explicit row-major layout despite the column-major default:
// each matrix is sized row-major (element stride 32), the two matrices occupy 60 bytes, so
// the trailing scalar lands at offset 60.
// REFLECT: "name": "values"
// REFLECT: "offset": 0, "size": 60, "elementStride": 32
// REFLECT: "name": "trailingValue"
// REFLECT: "offset": 60

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}
