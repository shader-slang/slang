//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=7d2dd721ab3c1d33497a937e79d182339817290b8e4a173ec5ce69d40d28ae8d
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
