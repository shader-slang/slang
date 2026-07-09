//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=9506ab20aaa39e5ee0d868d0a15106a005c8e09b8482625b171772602af36266
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
