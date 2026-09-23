//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=7b8fdedc5a4069b7a10da3f6dcaf8b3268c6f74fbb85dd4ea2edff960fab2b8b
//META: purpose=A GenericTypeConstraintDecl is only allowed as a child of an InterfaceDecl or a GenericDecl; `isDeclAllowed` rejects a `__constraint` written in a struct scope with declaration-not-allowed-here (E30102).
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the placement half of the `__constraint` rule, which runs before
// the This-subject check that constraint-subject-this-rejected.slang covers.
// The three accepted constraint spellings all land in an interface scope as
// requirement members; writing the dedicated `__constraint` keyword inside a
// `struct` instead puts the GenericTypeConstraintDecl in a container that is
// neither an interface nor a generic, so the parser's placement check rejects
// it. The CHECK pins E30102 at the declaration, which is what proves the
// rejection comes from placement rather than from the subject type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IBar
{
    int bar();
}

struct S
{
    __constraint int : IBar;
//CHECK:             ^ declaration not allowed here
//CHECK:             ^ GenericTypeConstraintDecl is not allowed here.
// CHECK: E30102
}

void main()
{
    printf("unreachable\n");
}
