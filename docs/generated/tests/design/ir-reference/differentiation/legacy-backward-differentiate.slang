//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#legacy-bridge
//META: doc_section_digest=9a4db90ca15d01457ccb4eccfe22540d5c691579087cfee38dbcfcddcd38b792
//META: purpose=LegacyBackwardDifferentiate carries three function operands read as apply_bwd, remat and propagate at positions 0, 1 and 2.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand shape that the legacy-bridge section singles out as not
// following the shared (targetFunc, legacyBwdDiffFunc) pattern of its siblings.
// The three phase requests synthesized for a `[Differentiable]` callable are
// captured as FileCheck variables in the order the dump emits them — primal,
// propagate, remat — and the final pattern requires the combined request to
// list them as apply_bwd first, remat second and propagate third, so a
// reordering of the operands would fail the test. The result type pinned on the
// same line is the old-style combined signature: void return, an inout
// differential pair, and the incoming adjoint.

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

// CHECK: let [[APPLY:%[A-Za-z0-9_]+]] :{{.*}}= BackwardDifferentiatePrimal(%f)
// CHECK: let [[PROP:%[A-Za-z0-9_]+]] :{{.*}}= BackwardDifferentiatePropagate(%f)
// CHECK: let [[REMAT:%[A-Za-z0-9_]+]] :{{.*}}= BackwardRemat(%f)
// CHECK: Func(Void, BorrowInOutParam(DiffPair(Float, {{%[A-Za-z0-9_]+}})), Float) = LegacyBackwardDifferentiate([[APPLY]], [[REMAT]], [[PROP]])
