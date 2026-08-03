//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00+00:00
//META: source_commit=60b1d48ca67a51f81b5a9103949a4934e6b1c33c
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#how-the-passes-are-ordered
//META: doc_section_digest=213ac6f0989d9adac2374fe9dcf3f5dff11ca62e30ad5f9de3fe117753c3b339
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
