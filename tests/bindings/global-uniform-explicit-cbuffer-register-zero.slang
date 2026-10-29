// Regression test for shader-slang/slang#14013.
//
// `cb` takes constant-buffer register 0 explicitly, so the default constant buffer
// that holds the global uniforms (`globalParams`) is allocated register 1. The HLSL
// and SPIR-V (with `-fvk-b-shift`) output must bind `globalParams` after `cb`, and
// reflection must report register 1 for the global scope's constant buffer.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SHIFT): -target spirv-asm -entry main -stage compute -fvk-b-shift 1000 0 -fvk-u-shift 2000 0
//TEST:REFLECTION(filecheck=REFL): -target hlsl -entry main -stage compute -no-codegen

// HLSL-DAG: cbuffer cb_0 : register(b0)
// HLSL-DAG: cbuffer globalParams_0 : register(b1)

// With `-fvk-b-shift`, both constant buffers keep their order after the shift.
// SHIFT-DAG: OpDecorate %cb Binding 1000
// SHIFT-DAG: OpDecorate %globalParams Binding 1001
// SHIFT-DAG: OpDecorate %result Binding 2000

// REFL: "globalScope": {
// REFL-NEXT: "kind": "constantBuffer",
// REFL-NEXT: "binding": {"kind": "constantBuffer", "index": 1},

struct P
{
    float4 x;
}

ConstantBuffer<P> cb : register(b0);
RWStructuredBuffer<float4> result : register(u0);
uniform float4 value;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    result[0] = cb.x + value;
}
