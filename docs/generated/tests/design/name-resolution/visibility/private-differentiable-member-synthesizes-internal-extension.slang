//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#synthesized-extensions-two-visibilities-not-one
//META: doc_section_digest=47affc0edb3ea7ecee55edf5aaf0b5ec2b9093a681b9b9cce224ebaa91f0d9de
//META: purpose=A `private` `[Differentiable]` member's synthesized derivative is attached to an extension that is `internal` rather than `private`, so the program checks and the derivative is usable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the split visibility for synthesis that produces an extension.
// `scaled` is `private` and `[Differentiable]`, so its derivative is
// synthesized onto an extension hoisted to module scope, where `private` is
// not a legal visibility at all (it is the shape that E30603 rejects when a
// user writes it). If the synthesized extension took the member's `private`
// visibility the file would not check. The buffer values assert both the
// primal (2*3+1 = 7) and the reverse-mode derivative with respect to k
// (dx = 2), which only exist if the synthesized derivative was attached and
// callable.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct S : IDifferentiable
{
    float x;

    [Differentiable]
    private float scaled(float k) { return x * k; }

    [Differentiable]
    float observe(float k) { return scaled(k) + 1.0; }

    __init(float v) { x = v; }
}

[Differentiable]
float wrapper(float k)
{
    S s = S(2.0);
    return s.observe(k);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var dp = diffPair(3.0, 0.0);
    bwd_diff(wrapper)(dp, 1.0);
    outputBuffer[0] = wrapper(3.0);
    outputBuffer[1] = dp.d;
}

// CHECK: 7.0
// CHECK-NEXT: 2.0
