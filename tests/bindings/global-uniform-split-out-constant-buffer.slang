// Regression test for shader-slang/slang#14013.
//
// `cb` takes constant-buffer register 0, so the default constant buffer `globalParams` gets
// register 1 and the `ConstantBuffer` field `s.c` of the global uniform `s` gets register 2.
// Legalization splits `s.c` out of `globalParams`. Because they share a resource kind,
// adding `globalParams`' binding again would shift `s.c` away from its reflected binding,
// so the emitted locations must match the ones reflection reports.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=MTL): -target metal -entry main -stage compute
//TEST:REFLECTION(filecheck=REFL): -target hlsl -entry main -stage compute -no-codegen

// HLSL-DAG: cbuffer cb_0 : register(b0)
// HLSL-DAG: cbuffer globalParams_0 : register(b1)
// HLSL-DAG: cbuffer globalParams_s_c_0 : register(b2)

// On Metal, `register(u1)` and the constant buffers share the `[[buffer]]` index space.
// MTL-DAG: cb_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}}
// MTL-DAG: result_{{[0-9]+}} {{\[\[}}buffer(1){{\]\]}}
// MTL-DAG: globalParams_{{[0-9]+}} {{\[\[}}buffer(2){{\]\]}}
// MTL-DAG: globalParams_s_c_{{[0-9]+}} {{\[\[}}buffer(3){{\]\]}}

// REFL: "name": "s",
// REFL-NEXT: "bindings": [
// REFL-NEXT: {"kind": "constantBuffer", "index": 2},
// REFL: "globalScope": {
// REFL-NEXT: "kind": "constantBuffer",
// REFL-NEXT: "binding": {"kind": "constantBuffer", "index": 1},

struct P
{
    float4 x;
}

struct S
{
    ConstantBuffer<P> c;
    float4 f;
}

ConstantBuffer<P> cb : register(b0);
RWStructuredBuffer<float4> result : register(u1);
uniform S s;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    result[0] = cb.x + s.c.x + s.f;
}
