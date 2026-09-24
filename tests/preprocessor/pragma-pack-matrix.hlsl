//TEST:REFLECTION(filecheck=REFLECT): -target hlsl -profile cs_6_0 -entry main -no-codegen -matrix-layout-column-major
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_0 -entry main -matrix-layout-column-major

// Before any source pragma, the command-line matrix layout remains the target default.
cbuffer BeforePragma : register(b0)
{
    float2x3 initialColumnMatrix;
    float initialColumnTail;
};

// Test native HLSL input, include inheritance, and state carried back to the caller.
#pragma pack_matrix(row_major)
#include "pragma-pack-matrix.h"

// Neither an inactive directive nor its malformed argument may change the default.
#if 0
#pragma pack_matrix(row_major)
#pragma pack_matrix(invalid)
#endif

cbuffer AfterInclude : register(b2)
{
    float2x3 columnMatrix;
    float columnTail;
};

// REFLECT: "name": "initialColumnTail"
// REFLECT: "offset": 40
// REFLECT: "name": "rowTail"
// REFLECT: "offset": 28
// REFLECT: "name": "columnTail"
// REFLECT: "offset": 40

// The existing HLSL lowering represents the concrete row-major matrix as two float3 rows.
// HLSL: float3 {{.*}}[int(2)];
// HLSL: {{.*}} rowMatrix
// HLSL: float2x3 columnMatrix

RWByteAddressBuffer output : register(u0);

[numthreads(1, 1, 1)]
void main()
{
    output.Store(
        0,
        asuint(
            initialColumnMatrix[0][0] + initialColumnTail + rowMatrix[0][0] + rowTail +
            columnMatrix[0][0] + columnTail));
}
