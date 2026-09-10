//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=A `[Differentiable]` annotation on an interface requirement becomes a requirement of the enclosing interface, so a generic bounded on that interface can be differentiated.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the attribute on a requirement is hoisted to the interface
// rather than staying nested under the member. `call` is generic over `IFn`
// and is itself differentiable; differentiating it forces the checker to find
// the differentiability requirement through the conformance witness of the
// bound type. CHECK asserts d(x*x)/dx at x=3.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IFn
{
    [Differentiable]
    float eval(float x);
}

struct Sq : IFn
{
    [Differentiable]
    float eval(float x)
    {
        return x * x;
    }
}

[Differentiable]
float call<T : IFn>(T t, float x)
{
    return t.eval(x);
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Sq s;
    var r = fwd_diff(call<Sq>)(s, diffPair(3.0, 1.0));
    outputBuffer[0] = r.d;
    // CHECK: 6.0
}
