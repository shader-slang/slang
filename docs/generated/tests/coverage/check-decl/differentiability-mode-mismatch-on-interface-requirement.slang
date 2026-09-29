//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-12T00:00:00+00:00
//META: source_commit=d592afa9b9efaf43740554b05a3fdf5b6f6e5077
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#differentiability-as-interface-conformance
//META: doc_section_digest=1fb413b84d197eac2789f533b613f130cab11653654d7878501604b76295e997
//META: purpose=Verifies a [ForwardDifferentiable] member cannot satisfy a [BackwardDifferentiable] interface requirement, and is rejected with E38110 naming the required mode.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of the FuncConstraintDecl arm of witness-table
// construction. Consider an interface that requires a backward-differentiable
// method, satisfied by a member that is only forward-differentiable:
//
//     interface IFoo { [BackwardDifferentiable] float f(float); }
//     struct Bad : IFoo { [ForwardDifferentiable] float f(float) {...} }
//
// The signatures match, so the structural requirement check passes and
// E38105 ("interface requirement mismatch") does not fire. What fails is the
// sibling constraint: header checking synthesizes a FuncConstraintDecl
// (slang-ast-decl.h) carrying sub = This.f, sup = IBackwardDifferentiableFunc<This.f>,
// and no witness can be found for it. That is the branch this test pins --
// reaching it requires a member that matches structurally but carries the
// wrong differentiability mode, which is why a plain non-[Differentiable]
// member (E38105) does not exercise it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

interface IFoo
{
    [BackwardDifferentiable]
    float f(float value);
//CHECK:  ^ see interface requirement declaration of 'f'
}

struct Bad : IFoo
{
    [ForwardDifferentiable]
    float f(float value) { return value * 2.0; }
//CHECK:  ^ E38110
//CHECK:  ^ member 'f' does not satisfy the required backward-differentiable constraint for interface requirement 'f'
}

[numthreads(1, 1, 1)]
void main() {}
