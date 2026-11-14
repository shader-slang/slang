//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=A `[Differentiable]` function gains an `IForwardDifferentiable` conformance whose synthesized `fwd_diff` member is found by ordinary lookup at the call site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that differentiability is recorded as a conformance rather than a
// modifier fact. `fwd_diff(sq)` is an ordinary member lookup on the function
// viewed as a type; it resolves only because checking synthesized the
// extension and its `fwd_diff` member. CHECK asserts the primal 9 and the
// derivative 6 = 2*3 that the synthesized member computes.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[Differentiable]
float sq(float x)
{
    return x * x;
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> r = fwd_diff(sq)(diffPair(3.0, 1.0));
    outputBuffer[0] = r.p;
    outputBuffer[1] = r.d;
    // CHECK: 9.0
    // CHECK: 6.0
}
