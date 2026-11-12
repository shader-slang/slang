//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=6399b7c2a0c9dbf90340a8eb7cb86baa3cd9c07289fb6d93436a49b19618df81
//META: purpose=A vector local loaded from a `StructuredBuffer<float3>` stays a native `float3` on Metal while the buffer element is `packed_float3`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's statement that a vector local is not a buffer
// element: in `float3 v = b[i];` the buffer element is `packed_float3` on
// Metal while `v` stays a native `float3`.

// The other shader-text targets are the control: none of them has a packed
// vector storage type, so `packed_float3` must not appear and the local is
// an ordinary three-component vector.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform StructuredBuffer<float3> b;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = b[tid.x];
    outbuf[0] = v.x + v.y + v.z;
}

// METAL: packed_float3
// METAL: float3 _S{{[0-9]+}} = float3(*(b_

// HLSL-NOT: packed_float3
// HLSL: float3
// GLSL-NOT: packed_float3
// GLSL: vec3
// SPV-NOT: packed_float3
// SPV: %v3float = OpTypeVector %float 3
// WGSL-NOT: packed_float3
// WGSL: vec3<f32>
