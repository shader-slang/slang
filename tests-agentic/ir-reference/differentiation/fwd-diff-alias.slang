//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/ir-reference/differentiation.md#forwarddifferentiate
//META: doc_section_digest=1de1b7063e4a6af82b1be7236c55a358b0eaaa925f91e6c745ca0f7700f9fadd
//META: purpose=The unprefixed fwd_diff(f) form also lowers to ForwardDifferentiate(%f), same as __fwd_diff.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `__fwd_diff(...)` as the AST origin of
// `ForwardDifferentiate`. The unprefixed alias `fwd_diff(...)` (also
// accepted by the parser) lowers to the same opcode.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fwd_diff(f)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: ForwardDifferentiate(%f)
