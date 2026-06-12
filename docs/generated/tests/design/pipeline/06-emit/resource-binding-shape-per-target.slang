//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Each backend renders an RWStructuredBuffer resource binding in its target-specific form: HLSL register(uN), GLSL layout(std430, binding=N), SPIR-V OpDecorate Binding/DescriptorSet, Metal buffer(N), WGSL @binding(N) @group(N).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A vk::binding(5, 2) is the source-level signal. Each backend turns
// it into its own resource-binding spelling. Metal does not honour
// vk::binding for buffer index selection (Metal assigns buffer
// indices positionally), so we only check that Metal emits *some*
// `buffer(N)` marker — not a specific N.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

[vk::binding(5, 2)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
// GLSL: layout(std430, binding = 5, set = 2) buffer
// SPIRV-DAG: OpDecorate %outBuf Binding 5
// SPIRV-DAG: OpDecorate %outBuf DescriptorSet 2
// METAL: buffer({{[0-9]+}})
// WGSL: @binding(5) @group(2){{.*}}var<storage, read_write> outBuf
