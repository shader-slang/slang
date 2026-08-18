//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#how-the-passes-are-ordered
//META: doc_section_digest=66d40862801105520d0f443657f6c0a7e1b9289bce092153a8e9060d2f1b77f5
//META: purpose=The post-link cleanup is carried by repeated `simplifyIR` fixed-point invocations (SSA / SCCP / SimplifyCFG / DCE) — at least two `### AFTER simplifyIR:` headers appear in the dump and the entry point survives every round.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the orchestrator's cleanup between major transformations is
// the simplifyIR fixed point rather than a one-shot pass: dumping around
// simplifyIR on a default SPIR-V compile shows the pass invoked at least
// twice, and the user entry point `func %main` is still present after the
// final observed round (cleanup simplifies without dropping live code).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after simplifyIR -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = a * 2 + 1;
}

// CHECK: ### AFTER simplifyIR:
// CHECK: ### AFTER simplifyIR:
// CHECK: func %main
