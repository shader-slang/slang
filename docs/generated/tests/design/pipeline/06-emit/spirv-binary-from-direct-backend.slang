//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#spir-v-direct
//META: doc_section_digest=42d7bd80dcaa52d3a64e75557163627a5a0662ae56a50368df92112952f8a0d1
//META: purpose=The direct SPIR-V backend emits a SPIR-V binary (no -asm) whose disassembly carries the same OpCapability/OpEntryPoint/OpExecutionMode/OpDecorate Binding markers as the assembly form.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `## Backends > SPIR-V (direct)` section names the direct
// SPIR-V emitter that "emits SPIR-V binary directly without going
// through GLSL". Pairs with the existing
// `spirv-asm-entry-point-and-decorate.slang` (which exercises the
// assembly form via `-target spirv-asm`); this test exercises the
// binary form (`-target spirv`) and pins the canonical text
// artifacts that appear in the disassembled binary.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry main -stage compute

[vk::binding(0, 0)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// Binary SPIR-V is disassembled for stdout; the same headline markers
// the assembly-form test pins must appear here too.
// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute %main "main"
// CHECK: OpExecutionMode %main LocalSize 4 1 1
// CHECK-DAG: OpDecorate %outBuf Binding 0
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 0
