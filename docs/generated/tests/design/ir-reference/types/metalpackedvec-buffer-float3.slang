//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=167b3150c6048e03cd650e8f68570073a489fbe77cab8ec27c015d56de4d32bd
//META: purpose=A float3 element of a Metal device buffer is rewritten to MetalPackedVec and emitted as MSL packed_float3.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal buffer-element-type legalization replaces the
// padded Vec element type of a device buffer with the tightly packed
// MetalPackedVec form. A StructuredBuffer<float3> global is read in
// the kernel so the buffer parameter survives to emit; METAL asserts
// the MSL declares the buffer with packed_float3 storage, which only
// appears if the element type was rewritten to MetalPackedVec(Float,
// 3).
//
// The rewrite is Metal-only, so the other text-emit back-ends pin the
// boundary of the claim: the same StructuredBuffer<float3> keeps an
// ordinary (padded) vector element type there — HLSL `float3`, GLSL
// `vec3`, WGSL `vec3<f32>`, and SPIR-V a runtime array of `%v3float`
// carrying the 16-byte `ArrayStride` that packing would have removed.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

StructuredBuffer<float3> positions;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 p = positions[tid.x];
    output[tid.x] = p.x + p.y + p.z;
}

// METAL: packed_float3 device* positions
// HLSL: StructuredBuffer<float3 > positions
// GLSL: vec3 _data[]
// WGSL: : array<vec3<f32>>
// SPIRV: OpDecorate %_runtimearr_v3float ArrayStride 16
// SPIRV: = OpTypeRuntimeArray %v3float
