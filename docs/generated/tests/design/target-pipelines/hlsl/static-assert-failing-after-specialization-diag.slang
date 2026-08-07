//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=bae02bfd9f0c35f37e81a96d88e88829e442c28a56e0f30d0bb769febf165f85
//META: purpose=Negative: checkStaticAssert (Phase B direct call) evaluates a static_assert whose condition depends on a generic value parameter after specialization; instantiating with a failing argument fires E41400.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `static_assert` is processed after specialization on the
// HLSL pipeline: the condition `N > 0` cannot be decided on the generic
// template itself, only on the specialized instance. Instantiating with
// N = 0 makes the specialized condition false, so the E41400 assertion
// failure carrying our message must be reported, with a note pointing
// at the specializing call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int pick<let N : int>()
{
    static_assert(N > 0, "N must be positive");
//CHECK:         ^ static assertion failed
//CHECK:         ^ static assertion failed, N must be positive
// CHECK: E41400
    return N * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pick<0>();
//CHECK:                   ^ see call to 'pick'
}
