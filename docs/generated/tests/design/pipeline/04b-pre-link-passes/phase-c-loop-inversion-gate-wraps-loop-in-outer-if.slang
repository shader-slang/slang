//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-c-mandatory-optimization-passes
//META: doc_section_digest=49feefd921e3cd705ce44340fcdfa68e653acc4e8992a6e7baac1b7332480921
//META: purpose=Phase C's invertLoops (C6) runs only under the LoopInversion toggle, moving the loop condition to the end of the loop and wrapping the loop in an outer if.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both the gate and the transform. The trip count comes from a
// `uniform`, so the loop cannot be unrolled or folded away. Without the
// flag the entry block branches straight into `loop(...)` and the
// condition is tested at the loop header. With `-loop-inversion` the entry
// block first computes the guard comparison and an `ifElse` around the
// whole loop, which is the "outer if" the transform is described by.

//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=INVERT):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -loop-inversion

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    for (int i = 0; i < n; i++) { s += i; }
    buf[0] = s;
}

// DEFAULT-LABEL: ### LOWER-TO-IR:
// DEFAULT-LABEL: func %main
// DEFAULT: param %tid
// DEFAULT-NEXT: loop(

// INVERT-LABEL: ### LOWER-TO-IR:
// INVERT-LABEL: func %main
// INVERT: param %tid
// INVERT-NEXT: cmpLT(0 : Int, %{{[A-Za-z0-9_]+}})
// INVERT-NEXT: ifElse(
// INVERT: loop(
