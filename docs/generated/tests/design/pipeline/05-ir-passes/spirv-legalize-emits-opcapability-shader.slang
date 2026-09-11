//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
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
