//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=79c80f93e7653bb012008aa7819b0c3ef28024e5d967ea6b1e4206f5d416adc5
//META: purpose=Expansion: spirv-legalize emits an `OpEntryPoint` and `OpExecutionMode LocalSize` for an alternative numthreads shape (8,4,1) — non-default localsize axis.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Numthreads axis on spirv-legalize: an explicit (8, 4, 1) layout
// surfaces in the `OpExecutionMode LocalSize` line. The pass must
// route the per-axis numthreads through to the SPIR-V emit.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// SPIRV: OpCapability Shader
// SPIRV: OpEntryPoint
// SPIRV: OpExecutionMode {{.*}} LocalSize 8 4 1
