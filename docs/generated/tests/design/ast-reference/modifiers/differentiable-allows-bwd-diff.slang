//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#differentiable-attribute-family
//META: doc_section_digest=df9843ec03c45db964221cccc955eef1e7d6d5189748b4ed076e3d1239bf9327
//META: purpose=The plain `[Differentiable]` spelling admits backward-mode differentiation, not only forward mode.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the class-to-spelling mapping the doc calls out as easy to get
// wrong: plain `[Differentiable]` is the backward-differentiable marker, so
// `bwd_diff` is available on it. The reverse-mode call propagates dOut=1 back
// into the pair at x=3 and CHECK asserts the accumulated 6 (2x) lands in the
// output buffer.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outBuf
RWStructuredBuffer<float> outBuf;

[Differentiable]
float square(float x)
{
    return x * x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> p = diffPair(3.0, 0.0);
    bwd_diff(square)(p, 1.0);
    outBuf[0] = p.d;
}
//CHECK: 6.0
