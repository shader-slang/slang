//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=2fbd38ac33b6c5e3c7d332612167499267fe9a58172bc83c495631475bead082
//META: purpose=ForwardDifferentiate has exactly one operand (baseFn): the user-named func value to differentiate.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `baseFn` as the single operand of `ForwardDifferentiate`.
// In the LOWER-TO-IR dump, the operand is the user-named `func %f`
// value (referenced as `%f`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float helperFunc(float x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(helperFunc)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: func %helperFunc
// CHECK: ForwardDifferentiate(%helperFunc)
