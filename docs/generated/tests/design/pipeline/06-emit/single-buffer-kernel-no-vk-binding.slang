//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Boundary: a single-buffer compute kernel with no explicit vk::binding annotation emits the default binding index per target — HLSL `register(u0)`, GLSL `binding = 0`, SPIR-V `Binding 0`, Metal `buffer(0)`, WGSL `@binding(0)`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: when no [vk::binding(N)] annotation is provided,
// the layout step assigns the default index 0. This pins the
// "default index = 0" behavior across every backend that has a
// resource-binding shape.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: register(u0)
// GLSL: binding = 0
// SPIRV-DAG: OpDecorate %outBuf Binding 0
// METAL: buffer(0)
// WGSL: @binding(0) @group(0)
