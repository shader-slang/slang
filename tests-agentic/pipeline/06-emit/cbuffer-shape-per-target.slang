//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=A Slang cbuffer is emitted in its target-shaped form on each text backend: HLSL `cbuffer`, GLSL `layout(std140) uniform`, Metal `constant *`, WGSL `var<uniform>`, SPIR-V Uniform/Block-decorated struct.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `cbuffer` in source is a per-target concept — it is a constant
// buffer on HLSL, a uniform block on GLSL, an Argument-Buffer-like
// `constant *` parameter on Metal, a uniform `var<uniform>` on WGSL,
// and a Block-decorated uniform struct on SPIR-V. The emit stage is
// responsible for translating the IR uniform-block into the right
// surface for each backend.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

[vk::binding(0, 0)]
cbuffer MyCB
{
    int v;
}

[vk::binding(1, 0)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = v + int(tid.x);
}

// HLSL: cbuffer MyCB_{{[0-9]+}} : register(b{{[0-9]+}})
// GLSL: layout(binding = 0)
// GLSL: layout(std140) uniform
// SPIRV-DAG: OpDecorate %MyCB Binding 0
// METAL: constant{{.*}}MyCB
// WGSL: var<uniform>{{.*}}MyCB
