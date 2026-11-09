//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=cb681a5a5428e160c2311cbf091e8eda34ecba068f868b779a464083304bb62f
//META: purpose=A `where T == U` clause is the equality spelling of a generic constraint and is satisfied when both parameters are inferred to the same type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `==` spelling of a `where` clause is accepted and
// satisfiable. The generic takes two independent type parameters constrained
// to be equal and is called with two `int` arguments; the CHECK asserts the
// body's marker value, which only prints if the equality constraint was
// discharged.

//TEST:INTERPRET(filecheck=CHECK):

int sameType<T, U>(T a, U b) where T == U { return 7; }

void main()
{
    //CHECK: 7
    printf("%d\n", sameType(1, 2));
}
