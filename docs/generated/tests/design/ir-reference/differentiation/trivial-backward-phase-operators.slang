//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#reverse-mode
//META: doc_section_digest=e69ab230c20c4f232ce87275aa033089eef9df01efc5d5cb733c5d15d1290426
//META: purpose=A [TreatAsDifferentiable] callable gets the trivial reverse-mode phases TrivialBackwardDifferentiatePrimal, TrivialBackwardDifferentiatePropagate and TrivialBackwardRemat instead of the ordinary ones.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the trivial-adjoint half of the reverse-mode table. The shader is
// the same shape as the ordinary reverse-mode case except that `g` carries
// `[TreatAsDifferentiable]`, so checking synthesizes the trivial phases. The
// CHECK-DAG patterns require all three trivial phases over `%g`, and the
// bounded CHECK-NOT lines require that none of the non-trivial spellings is
// also emitted for `%g` inside the LOWER-TO-IR section — the two families are
// alternatives, not layers.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[TreatAsDifferentiable]
float g(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> dp = DifferentialPair<float>(a, 0.0);
    __bwd_diff(g)(dp, 1.0);
    buf[0] = dp.d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK-DAG: = TrivialBackwardDifferentiatePrimal(%g)
// CHECK-DAG: = TrivialBackwardDifferentiatePropagate(%g)
// CHECK-DAG: = TrivialBackwardRemat(%g)
// CHECK-NOT: = BackwardDifferentiatePrimal(%g)
// CHECK-NOT: = BackwardDifferentiatePropagate(%g)
// CHECK-NOT: = BackwardRemat(%g)
// CHECK: {{^}}###{{$}}
