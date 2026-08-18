//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-construction
//META: doc_section_digest=94baa99b60cb82fe486d311fcf1a6ed19cf37b8c6ff65ecb372a31812c6e6208
//META: purpose=The DifferentialPair<T> constructor lowers to MakeDiffPair whose two operands are the primal and the differential, in that order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `DifferentialPair<T>.__init` is the user surface of the
// `MakeDiffPair` opcode and that its operand order is (primal, differential).
// Both operands are `uniform` globals so the constant folder cannot collapse
// the construction and the dump keeps the two named values; CHECK pins the
// line inside `func %main` with `%a` first and `%d` second, plus the
// `DiffPair(Float, ...)` result type that identifies the value-typed flavour
// of the family as opposed to the pointer-typed `MakeDiffRefPair`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, d)).d;
}

// CHECK: func %main
// CHECK: DiffPair(Float, {{%[A-Za-z0-9_]+}}){{.*}}= MakeDiffPair(%a, %d)
