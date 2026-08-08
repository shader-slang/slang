//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#backwarddifferentiateprimal-backwardremat-and-backwarddifferentiatepropagate
//META: doc_section_digest=c28d9ea571c3dddec6c979570fa60f8acee16db5827841e6b2f72c56fcfd768d
//META: purpose=A [Differentiable] callable reaches lowering as a conformance whose apply_bwd, propagate and remat members are BackwardDifferentiatePrimal, BackwardDifferentiatePropagate and BackwardRemat requests over the base function.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that differentiability arrives at lowering as an interface
// conformance carrying three function-valued phases rather than as a flag on
// the function. Marking `f` as `[Differentiable]` is the only differentiation
// surface in the shader; the ordered CHECKs then require the three synthesized
// members to appear as separate module-scope requests, each taking `%f` as its
// single operand. Each pattern also pins the phase's own signature — the primal
// phase returns a tuple ending in the minimal-context type, the propagate phase
// consumes the full intermediate-context type, and remat maps the full context
// plus the minimal context back to the primal result — which is what makes the
// three rows distinguishable from one another.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> dp = DifferentialPair<float>(a, 0.0);
    __bwd_diff(f)(dp, 1.0);
    buf[0] = dp.d;
}

// CHECK: func %f{{.*}}: Func(Float, Float)
// CHECK: tuple_type(Float, BackwardDiffMinimalContextType(%f)){{.*}}= BackwardDifferentiatePrimal(%f)
// CHECK: BackwardDiffIntermediateContextType(%f){{.*}}= BackwardDifferentiatePropagate(%f)
// CHECK: BackwardDiffIntermediateContextType(%f), BackwardDiffMinimalContextType(%f){{.*}}= BackwardRemat(%f)
