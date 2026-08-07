//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=5c32316c757a32af0aa6a61ee5f325f4830e8f73bc176d8904fdb4ddc1d8d758
//META: purpose=A `__constraint` whose subject resolves to the bare `This` type is rejected with constraint-subject-cannot-be-this-type (E30427); giving an interface a base is the inheritance clause's role.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented rejection of a bare-This constraint subject:
// the relocated GenericTypeConstraintDecl lands in the interface scope
// but the header visitor must diagnose E30427 instead of accepting a
// constraint that plays the inheritance clause's role.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IBar
{
    int bar();
}

interface IFoo
{
    __constraint This : IBar;
//CHECK:         ^^^^ the subject of a constraint cannot be the 'This' type
//CHECK:         ^^^^ constrain an associated type such as 'This.A' here, or use an inheritance clause (e.g. 'interface IFoo : IBar') to give an interface a base
// CHECK: E30427
}
