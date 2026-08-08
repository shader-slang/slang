//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
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
