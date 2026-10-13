//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#phase-skeleton-of-linkandoptimizeir
//META: doc_section_digest=106c302803b4e0e47a157751f5c27d76cf9a04789ad4a6bfb7ac6f7c5c426390
//META: purpose=`performForceInlining` is one of the passes the phase skeleton says appears more than once, so a single SPIR-V compile produces at least two of its dump stages.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the phase-skeleton's "several passes appear more than once"
// claim for `performForceInlining`, which the table lists in both the
// stage-switch region and the late-cleanup region. Dumping only that
// pass and requiring two `### AFTER performForceInlining:` labels in
// sequence proves the orchestrator invoked it at least twice; a single
// invocation would leave the second CHECK-LABEL unmatched.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after performForceInlining -o - -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int bump(int x)
{
    return x + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bump(int(tid.x));
}

// CHECK-LABEL: ### AFTER performForceInlining:
// CHECK-LABEL: ### AFTER performForceInlining:
