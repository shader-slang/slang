//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=Boundary: a `[ForceUnroll]` loop with zero static iterations is removed entirely by the loop-unroll pass and emits a documented `E30514` warning.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Zero-iter edge of `[ForceUnroll]`. The loop-unroll pass discards
// the body entirely and the diagnostic surface reports `E30514: loop
// runs zero times`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    [ForceUnroll]
    for (int i = 0; i < 0; ++i)
/*CHECK:
    ^^^ the loop runs for 0 iterations and will be removed.
*/
// CHECK: E30514
    {
        buf[i] = i;
    }
}
