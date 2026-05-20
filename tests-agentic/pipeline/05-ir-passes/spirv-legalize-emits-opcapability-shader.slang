//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
//META: purpose=The SPIR-V legalize pass ensures the emitted SPIR-V module carries `OpCapability Shader`, `OpEntryPoint`, and an `OpExecutionMode LocalSize` decoration.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "SPIR-V legalize" entry in the target-specific lowering table
// (`slang-ir-spirv-legalize.cpp`) is responsible for adjusting the
// IR shape into something the SPIR-V emitter can consume. The
// observable footprint is the standard SPIR-V preamble:
// `OpCapability Shader`, `OpEntryPoint GLCompute`, and an
// `OpExecutionMode ... LocalSize` for the entry point.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 7;
}

// SPIRV: OpCapability Shader
// SPIRV: OpEntryPoint GLCompute
// SPIRV: OpExecutionMode {{.*}} LocalSize 4 1 1
