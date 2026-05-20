//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/ir-reference/differentiation.md#forwarddifferentiate
//META: doc_section_digest=1de1b7063e4a6af82b1be7236c55a358b0eaaa925f91e6c745ca0f7700f9fadd
//META: purpose=ForwardDifferentiate is hoistable so two syntactic __fwd_diff(f) occurrences on the same base function dedupe to one ForwardDifferentiate(%f) IR value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ForwardDifferentiate` is hoistable (the doc's `Flags` column shows
// `H`). Two source references to `__fwd_diff(f)` on the same base
// function dedupe to a single `ForwardDifferentiate(%f)` declaration
// in the LOWER-TO-IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float b;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    // Two __fwd_diff(f) references should dedupe to one IR value.
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, 1.0)).d;
    buf[1] = __fwd_diff(f)(DifferentialPair<float>(b, 1.0)).d;
}

// Two __fwd_diff(f) source references dedupe: both call sites in
// `func %main` route through the same hoistable `%fwdx5Fdiff`
// callee, which is the single `ForwardDifferentiate(%f)` binding.
// CHECK: ForwardDifferentiate(%f)
// CHECK: func %main
// CHECK: call %fwdx5Fdiff
// CHECK: call %fwdx5Fdiff
