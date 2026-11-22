// Regression for shader-slang/slang#12990: an explicit `row_major` on an array-of-matrix
// cbuffer member is honored.

//TEST:REFLECTION(filecheck=REFLECT):-profile cs_6_0 -target hlsl -no-codegen -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -profile cs_6_0 -entry main -matrix-layout-column-major

cbuffer Params
{
    row_major float2x3 values[2];
    float trailingValue;
}

// The array element keeps its explicit row-major layout despite the column-major default.
// A `row_major float2x3` lays its two rows on 16-byte boundaries, so the array element stride
// is 32 while the element's actual data ends after 28 bytes; the array size reclaims the last
// element's trailing padding: (2-1)*32 + 28 = 60. The trailing scalar then packs at offset 60.
// REFLECT: "name": "values"
// REFLECT: "offset": 0, "size": 60, "elementStride": 32
// REFLECT: "name": "trailingValue"
// REFLECT: "offset": 60

// The layout also reaches the SPIR-V element decorations: Slang `row_major` maps to SPIR-V
// `ColMajor` (the convention is inverted), so the element is decorated `ColMajor` rather than the
// `RowMajor` the column-major default would otherwise apply.
// SPV: OpMemberDecorate {{.*}} ColMajor
// SPV: OpMemberDecorate {{.*}} MatrixStride 16

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = values[0][0][0] + trailingValue;
}
