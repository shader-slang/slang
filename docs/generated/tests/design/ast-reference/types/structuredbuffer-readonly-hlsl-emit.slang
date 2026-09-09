//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=An HLSLStructuredBufferType `StructuredBuffer<T>` is a distinct read-only buffer type from its RW sibling and keeps that spelling through HLSL emission.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the read-only and read-write structured buffer families
// are separate types rather than one type with a flag: the same shader
// declares both and copies between them. CHECK anchors the read-only
// spelling at line start so it cannot be satisfied by the `RW` prefix
// of its sibling.
//
// The other text-emit back-ends spell the read-only half with their own
// access qualifier: GLSL adds `readonly` to the block, SPIR-V decorates
// the variable `NonWritable`, and WGSL declares it `var<storage, read>`
// against the sibling's `read_write`. Metal has no such qualifier — it
// lowers both halves to an unqualified `device` pointer — so the Metal
// CHECK pins that both buffers still arrive as separate device-address-
// space parameters rather than asserting a read-only marker that the
// back-end does not emit.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

StructuredBuffer<int> g_in;
RWStructuredBuffer<int> g_out;

//CHECK-DAG: {{^}}StructuredBuffer<int
//CHECK-DAG: RWStructuredBuffer<int

//GLSL-DAG: binding = 0) readonly buffer
//GLSL-DAG: binding = 1) buffer

//METAL-DAG: int device* g_in_{{[0-9]+}} {{\[\[}}buffer(0)
//METAL-DAG: int device* g_out_{{[0-9]+}} {{\[\[}}buffer(1)

//SPIRV-DAG: OpDecorate %g_in NonWritable
//SPIRV-DAG: %StructuredBuffer = OpTypeStruct %_runtimearr_int
//SPIRV-DAG: %RWStructuredBuffer = OpTypeStruct %_runtimearr_int

//WGSL-DAG: var<storage, read> g_in_0 : array<i32>
//WGSL-DAG: var<storage, read_write> g_out_0 : array<i32>
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_out[tid.x] = g_in[tid.x];
}
