//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#option-set-toggles
//META: doc_section_digest=7c951a88b67c0d94a6d4504f2d7616fdfe324b58f96f2245c67178d693892a88
//META: purpose=The minimum-optimizations toggle gates C3 and C4, so the empty-block cleanup that normally merges the entry block away is skipped when it is set.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the gate rather than the pass. The same shader is compiled
// twice; C1 and C2 run either way, so the untaken arm is gone in both
// snapshots. What differs is the leftover block structure: by default the
// entry block flows straight into the buffer store, whereas with
// `-minimum-slang-optimization` the CFG cleanup is skipped and the entry
// block still ends in a bare `unconditionalBranch` to the block holding
// the store. The `-NEXT` CHECKs pin exactly that one-line difference.

//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=MINOPT):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -minimum-slang-optimization

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (true) { buf[0] = a; }
    else { buf[0] = 9999; }
}

// DEFAULT-LABEL: ### LOWER-TO-IR:
// DEFAULT-LABEL: func %main
// DEFAULT: param %tid
// DEFAULT-NEXT: rwstructuredBufferGetElementPtr

// MINOPT-LABEL: ### LOWER-TO-IR:
// MINOPT-LABEL: func %main
// MINOPT: param %tid
// MINOPT-NEXT: unconditionalBranch
