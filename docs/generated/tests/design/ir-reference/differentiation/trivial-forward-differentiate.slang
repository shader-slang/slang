//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forward-mode
//META: doc_section_digest=ff7339aef6c3ffedb9466f9df10a8adb3a5ae02fe094735a717bd998cd8bc62a
//META: purpose=A [TreatAsDifferentiable] function gets a synthesized fwd_diff member built from TrivialForwardDifferentiate instead of ForwardDifferentiate.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second forward-mode row: the trivial derivative request, whose
// AST origin is the synthesized `fwd_diff` member that checking creates for a
// `[TreatAsDifferentiable]` callable. The same `__fwd_diff(g)` surface is used
// as in the ordinary case, so the only difference is the attribute on `g`.
// CHECK requires the synthesized member to carry the JVP function type and to
// be produced by `TrivialForwardDifferentiate(%g)`, and forbids the
// non-trivial `ForwardDifferentiate(%g)` spelling within the LOWER-TO-IR
// section, which is what distinguishes the two rows.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[TreatAsDifferentiable]
float g(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(g)(DifferentialPair<float>(a, d)).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: let {{%[A-Za-z0-9_]+}} : Func(DiffPair(Float, {{%[A-Za-z0-9_]+}}), DiffPair(Float, {{%[A-Za-z0-9_]+}})) = TrivialForwardDifferentiate(%g)
// CHECK-NOT: = ForwardDifferentiate(%g)
// CHECK: {{^}}###{{$}}
