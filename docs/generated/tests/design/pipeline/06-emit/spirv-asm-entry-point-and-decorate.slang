//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#spir-v-direct
//META: doc_section_digest=42d7bd80dcaa52d3a64e75557163627a5a0662ae56a50368df92112952f8a0d1
//META: purpose=The direct SPIR-V backend emits SPIR-V text with OpCapability Shader, OpEntryPoint GLCompute, OpExecutionMode LocalSize, and OpDecorate Binding/DescriptorSet for a buffer resource.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The direct SPIR-V backend (slang-emit-spirv.cpp) emits SPIR-V
// assembly that includes the standard Shader capability, an entry
// point with the GLCompute execution model, a LocalSize execution
// mode for [numthreads(...)], and Binding/DescriptorSet decorations
// for a buffer resource. These are textual markers that the doc lists
// as defining characteristics of SPIR-V output.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[vk::binding(3, 1)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpCapability Shader
// CHECK: OpEntryPoint GLCompute %main "main"
// CHECK: OpExecutionMode %main LocalSize 8 1 1
// CHECK-DAG: OpDecorate %outBuf Binding 3
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 1
