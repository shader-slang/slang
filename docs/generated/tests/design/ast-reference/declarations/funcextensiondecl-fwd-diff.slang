//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#funcextensiondecl
//META: doc_section_digest=23b3247cc6a5a19e7e7455ea0b4ffac555ac3ebd72721db7a1554013d44d65dd
//META: purpose=A `__func_extension fwd_diff(f)(...)` attaches a custom forward derivative to an existing function without modifying its definition.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `__func_extension` shorthand supplies a derivative for a
// function that is not itself marked differentiable. The target function has
// no `[Differentiable]` attribute, so `fwd_diff(cube)` can only resolve
// through the attached extension; the CHECK asserts both the primal and the
// derivative value the extension body computes.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature

float cube(float x) { return x * x * x; }

__func_extension fwd_diff(cube)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(cube(x.p), 3.0 * x.p * x.p * x.d);
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpx = diffPair(2.0, 1.0);
    DifferentialPair<float> result = fwd_diff(cube)(dpx);
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 8.0
    // CHECK: 12.0
}
