//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=The same `[Differentiable]` attribute also supplies the backward-differentiable conformance, so `bwd_diff` resolves through ordinary lookup and propagates into the pair.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second of the pair of conformances the attribute installs.
// `bwd_diff(sq)` writes the accumulated derivative back into the input pair,
// which only happens if the backward interface's `apply_bwd` requirement was
// satisfied by a synthesized member. CHECK asserts d(x*x)/dx at x=3 seeded
// with 1.0, i.e. 6.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[Differentiable]
float sq(float x)
{
    return x * x;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> dp = diffPair(3.0, 0.0);
    bwd_diff(sq)(dp, 1.0);
    outputBuffer[0] = dp.d;
    // CHECK: 6.0
}
