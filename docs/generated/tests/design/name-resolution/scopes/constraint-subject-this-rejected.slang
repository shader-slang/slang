//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
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
