//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#spir-v-direct
//META: doc_section_digest=aae1d137ae4ede6a4d8e6578bad9c5e5a3aaf69e70a51b3761ca9368e130bab4
//META: purpose=The direct SPIR-V backend emits a complete SPIR-V module without going through GLSL, carrying `OpCapability Shader`, `OpEntryPoint GLCompute`, `OpExecutionMode ... LocalSize`, and `OpDecorate ... Binding`/`DescriptorSet` for a buffer resource.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the direct SPIR-V path produces a self-contained module. The
// GLSL-mediated route would leave glslang's fingerprint (`; Generator:
// Khronos Glslang`), so CHECK pins the Slang generator string alongside the
// structural markers a compute module must have. `OpDecorate ... Binding 0`
// and `DescriptorSet 0` prove the resource binding survived into SPIR-V
// annotations rather than a GLSL `layout(...)` string.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// SPIRV-DAG: ; Generator: Khronos Slang Compiler
// SPIRV-DAG: OpCapability Shader
// SPIRV-DAG: OpEntryPoint GLCompute %main "main"
// SPIRV-DAG: OpExecutionMode %main LocalSize 64 1 1
// SPIRV-DAG: OpDecorate %outBuf Binding 0
// SPIRV-DAG: OpDecorate %outBuf DescriptorSet 0
