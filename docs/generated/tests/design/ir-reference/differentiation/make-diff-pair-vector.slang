//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-construction
//META: doc_section_digest=94baa99b60cb82fe486d311fcf1a6ed19cf37b8c6ff65ecb372a31812c6e6208
//META: purpose=MakeDiffPair carries a differential-pair result type over a vector element type, spelled DiffPair(Vec(Float, 3), witness).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the element-type axis of the construction claim: the opcode bundles
// "a value-typed primal with its differential" for any `T` that has an
// `IDifferentiable` conformance, not only for a scalar. Building the pair from
// two `uniform float3` globals makes the dump print the result type as
// `DiffPair(Vec(Float, 3 : Int), <witness>)`; CHECK pins that spelling on the
// same line as the `MakeDiffPair(%a, %d)` producer so the element type and the
// producing opcode are asserted together.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float3 a;
uniform float3 d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float3 f(float3 x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float3>(a, d)).d.x;
}

// CHECK: func %main
// CHECK: DiffPair(Vec(Float, 3 : Int),{{.*}}= MakeDiffPair(%a, %d)
