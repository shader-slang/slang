//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=87eaa476e05e936a2bb0b7cd0ad066839d24b61d75d234809ccf6e876c8763e3
//META: purpose=A `__constraint` outside an interface or generic (here, in a struct body) is rejected by the central decl-nesting check with E30102.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented central legality rule: the constraint decl
// a `__constraint` produces is only permitted as a member of an
// interface or generic. Placing one in a struct body is diagnosed as
// a misplaced declaration; CHECK pins the code E30102.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IFoo
{
    int f();
}

struct Bad
{
    __constraint int : IFoo;
//CHECK:             ^ error E30102
//CHECK:             ^ GenericTypeConstraintDecl is not allowed here.
}
