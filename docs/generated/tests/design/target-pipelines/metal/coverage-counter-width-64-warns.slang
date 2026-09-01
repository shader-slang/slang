//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metal-caps-coverage-counters-to-32-bits
//META: doc_section_digest=bdf089cd96aae34d768b3ab1a8777280f191af87dae6923fd22a599aad29aeaa
//META: purpose=Negative: an explicitly requested 64-bit coverage counter width on Metal is capped with warning E45115 rather than honored.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the cap is silent only for the implicit default: when the
// caller spells out `-trace-coverage-counter-width 64`, Slang reports
// `CoverageCounterWidthCappedForMetal` so the choice is visibly not
// honored. The test pins the stable diagnostic codes rather than the long
// message text: E45115 is the cap warning itself and E40100 is the
// entry-point rename that every Metal compile of a `main` emits, so both
// diagnostics the run produces are accounted for.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage compute -trace-coverage -trace-coverage-counter-width 64

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x);
}

//CHECK: E45115
//CHECK: E40100
