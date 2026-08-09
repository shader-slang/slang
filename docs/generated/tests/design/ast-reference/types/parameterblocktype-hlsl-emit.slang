//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=A ParameterBlockType `ParameterBlock<T>` groups its element's fields into one binding whose members are reachable by field access at the entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parameter-block leaf of the uniform parameter-group
// family: the block's element struct is accessed by ordinary field
// syntax, and the block becomes its own binding in the emitted HLSL.
// CHECK matches the cbuffer the block lowers to plus the field read, so
// a block that had been flattened into loose globals would not match.
// On the other back-ends the same grouping shows up as the block taking
// a descriptor set / bind group of its own — set 1 / group(1), distinct
// from the storage buffer's set 0 — with the element's fields still
// reached by ordinary member access off that one binding.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

struct Material { float4 color; float roughness; }

ParameterBlock<Material> g_mat;
RWStructuredBuffer<float4> g_out;

//CHECK-DAG: cbuffer
//CHECK-DAG: color_{{[0-9]+}}

//GLSL: layout(binding = 0, set = 1)
//GLSL-NEXT: layout(std140) uniform block_Material_0
//GLSL: g_mat_0.color_0 * g_mat_0.roughness_0

//METAL: Material_{{.*}} constant* g_mat
//METAL: g_mat_{{[0-9]+}}->color_0

//SPIRV-DAG: OpDecorate %g_mat DescriptorSet 1
//SPIRV-DAG: %g_mat = OpVariable %_ptr_Uniform_Material_std140 Uniform

//WGSL: @binding(0) @group(1) var<uniform> g_mat_0 : Material_std140_0
//WGSL: g_mat_0.color_0
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_mat.color * g_mat.roughness;
}
