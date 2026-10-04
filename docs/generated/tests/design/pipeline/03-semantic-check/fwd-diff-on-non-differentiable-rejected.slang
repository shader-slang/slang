//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=Asking for `fwd_diff` of a function with no differentiable conformance fails as a member lookup (E30027), which is how the subtype query surfaces to the user.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative consequence of modelling differentiability as a
// conformance: with no attribute and no explicit extension, nothing installs
// the interface, so `fwd_diff` is simply not a member of the function. CHECK
// pins E30027 naming `fwd_diff` and the callee.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl

float plain(float x)
{
    return x * x;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    var r = fwd_diff(plain)(diffPair(2.0, 1.0));
//CHECK: E30027
    outputBuffer[0] = r.d;
}
