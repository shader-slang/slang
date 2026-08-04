//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#how-the-passes-are-ordered
//META: doc_section_digest=213ac6f0989d9adac2374fe9dcf3f5dff11ca62e30ad5f9de3fe117753c3b339
//META: purpose=The DCE / cleanup category runs multiple times during `linkAndOptimizeIR`; multiple `### AFTER eliminateDeadCode:` headers appear in the dump.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The orchestrator (`linkAndOptimizeIR`) interleaves multiple
// invocations of cleanup passes. We observe that two distinct
// `### AFTER eliminateDeadCode:` headers exist in a default
// SPIR-V dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after eliminateDeadCode -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CHECK: ### AFTER eliminateDeadCode:
// CHECK: ### AFTER eliminateDeadCode:
