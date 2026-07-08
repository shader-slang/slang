//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:41:39+00:00
//META: source_commit=199a52d71d360a84e8a13837b913746113a9b510
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=88de88b16c165f633a8ad6ade9438213eb05e41453c58f2679fe7e345b5df561
//META: purpose=Negative: checkStaticAssert (Phase B direct call) evaluates a static_assert whose condition depends on a generic value parameter after specialization; instantiating with a failing argument fires E41402.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `static_assert` is processed after specialization on the
// HLSL pipeline: the condition `N > 0` cannot be decided on the generic
// template itself, only on the specialized instance. Instantiating with
// N = 0 makes the specialized condition false, so the E41402 assertion
// failure carrying our message must be reported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int pick<let N : int>()
{
    static_assert(N > 0, "N must be positive");
    //CHECK: N must be positive
    // CHECK: E41402
    return N * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pick<0>();
}
