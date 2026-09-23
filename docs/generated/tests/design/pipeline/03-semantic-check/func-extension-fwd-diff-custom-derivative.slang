//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=The explicit `__func_extension fwd_diff(f)(...)` spelling is rewritten into the same conformance-plus-member representation, so `fwd_diff(f)` resolves to the user's body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the explicit spelling lands on the same representation as the
// attribute path. `cube` carries no `[Differentiable]` attribute, so
// `fwd_diff(cube)` can only resolve through the extension the declaration is
// rewritten into. CHECK asserts the primal 8 and the derivative 12 that the
// hand-written body computes for x = 2.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

float cube(float x)
{
    return x * x * x;
}

__func_extension fwd_diff(cube)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(cube(x.p), 3.0 * x.p * x.p * x.d);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    var r = fwd_diff(cube)(diffPair(2.0, 1.0));
    outputBuffer[0] = r.p;
    outputBuffer[1] = r.d;
    // CHECK: 8.0
    // CHECK: 12.0
}
