//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Stress: four resource bindings get four distinct default indices in the emit — HLSL u0..u3, GLSL binding=0..3, WGSL @binding(0..3), SPIR-V Binding 0..3.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of the resource-binding shape: rather than a single
// buffer (which gets index 0 trivially), four sibling
// RWStructuredBuffer parameters must get four distinct default
// indices. This pressures the layout-to-emit boundary on every
// target that has a binding-index surface.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> a;
RWStructuredBuffer<int> b;
RWStructuredBuffer<int> c;
RWStructuredBuffer<int> d;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    a[tid.x] = 1;
    b[tid.x] = 2;
    c[tid.x] = 3;
    d[tid.x] = 4;
}

// HLSL-DAG: a_{{[0-9]+}} : register(u0)
// HLSL-DAG: b_{{[0-9]+}} : register(u1)
// HLSL-DAG: c_{{[0-9]+}} : register(u2)
// HLSL-DAG: d_{{[0-9]+}} : register(u3)

// GLSL-DAG: binding = 0
// GLSL-DAG: binding = 1
// GLSL-DAG: binding = 2
// GLSL-DAG: binding = 3

// SPIRV-DAG: OpDecorate %a Binding 0
// SPIRV-DAG: OpDecorate %b Binding 1
// SPIRV-DAG: OpDecorate %c Binding 2
// SPIRV-DAG: OpDecorate %d Binding 3

// WGSL-DAG: @binding(0) @group(0){{.*}}var<storage, read_write> a
// WGSL-DAG: @binding(1) @group(0){{.*}}var<storage, read_write> b
// WGSL-DAG: @binding(2) @group(0){{.*}}var<storage, read_write> c
// WGSL-DAG: @binding(3) @group(0){{.*}}var<storage, read_write> d
