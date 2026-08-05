//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=167b3150c6048e03cd650e8f68570073a489fbe77cab8ec27c015d56de4d32bd
//META: purpose=Expansion: a float3 field of a struct used as a Metal device-buffer element is rewritten to packed_float3 storage.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the buffer-element rewrite reaches vector fields nested
// inside a struct element, not just direct vector elements: rewriting
// the element type of a Metal device buffer must repack the struct's
// contents. The kernel reads both fields of a StructuredBuffer<Item>
// element; METAL asserts the emitted struct declaration carries a
// packed_float3 field.
//
// Repacking the struct's contents is Metal-only, so the other
// text-emit back-ends pin the boundary of the claim: the same Item
// element keeps an ordinary vector field — HLSL `float3 dir`, GLSL
// `vec3 dir`, WGSL a `vec3<f32>` field in the std430 struct, and
// SPIR-V an `OpTypeStruct %v3float %float`.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

struct Item
{
    float3 dir;
    float weight;
};

StructuredBuffer<Item> items;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Item it = items[tid.x];
    output[tid.x] = it.dir.x * it.weight;
}

// METAL-DAG: struct Item
// METAL-DAG: packed_float3 dir
// HLSL-DAG: struct Item
// HLSL-DAG: float3 dir
// GLSL-DAG: struct Item
// GLSL-DAG: vec3 dir
// WGSL-DAG: struct Item
// WGSL-DAG: dir{{.*}} : vec3<f32>
// SPIRV-DAG: = OpTypeStruct %v3float %float
