//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=Target-specific lowering passes (slang-ir-hlsl-legalize, slang-ir-glsl-legalize, slang-ir-spirv-legalize, slang-ir-metal-legalize, slang-ir-wgsl-legalize) translate a generic resource binding into the target's native form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[vk::binding(N, S)]` is the source-level signal for binding N in
// descriptor set S. Each backend's target-specific legalization pass
// (named in the doc's "How target choice affects IR" section) turns
// it into the target's native form. Metal does not honour
// `vk::binding(N)` for buffer-index selection — Metal indices are
// positional from the entry-point parameter list — so the Metal
// check accepts any `buffer({{[0-9]+}})`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

[vk::binding(3, 1)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
// GLSL: layout(std430, binding = 3, set = 1) buffer
// SPIRV-DAG: OpDecorate %outBuf Binding 3
// SPIRV-DAG: OpDecorate %outBuf DescriptorSet 1
// METAL: buffer({{[0-9]+}})
// WGSL: @binding(3) @group(1){{.*}}var<storage, read_write> outBuf
