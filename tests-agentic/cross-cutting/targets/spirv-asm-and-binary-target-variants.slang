//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#targets
//META: doc_section_digest=e324a10260c794d77c45954ccddaf61cf66af2794e97a8289f9655330ee625d4
//META: purpose=The Targets table lists `SLANG_SPIRV` and `SLANG_SPIRV_ASM` as variants of the same emit backend; both `-target spirv-asm` produces SPIR-V text (the assembly-text-form observable in the no-GPU runner).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Targets table groups `SLANG_SPIRV` (binary) and
// `SLANG_SPIRV_ASM` (assembly text) under the SPIR-V direct emit
// backend. The assembly form is what the no-GPU runner can FileCheck.
// We exercise the same source on `-target spirv-asm` and assert
// SPIR-V-shape header markers — `OpCapability Shader`,
// `OpMemoryModel`, and `OpEntryPoint GLCompute` — appear.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// SPIRV: OpCapability Shader
// SPIRV: OpMemoryModel
// SPIRV: OpEntryPoint GLCompute %main "main"
// SPIRV: OpExecutionMode %main LocalSize 1 1 1
