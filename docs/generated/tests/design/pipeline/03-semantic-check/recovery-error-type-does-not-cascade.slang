//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=4c9f71a256907b7a79759ca00a313ad651ac24d85015af4f3fe1673fd66d9dde
//META: purpose=Check-level recovery substitutes an error type for an unresolved name, so a second, unrelated error in the next statement is still reported.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that recovery is "continue with a placeholder type" rather than
// abort. Two independent unresolved names appear in consecutive statements;
// if the first failure poisoned checking, the second would never be reached.
// CHECK pins one E30015 per statement, and exhaustive mode makes "exactly
// these two, nothing swallowed and nothing cascaded" part of the assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int a = undefinedOne;
//CHECK: E30015
//CHECK: undefined identifier 'undefinedOne'.
    int b = undefinedTwo;
//CHECK: E30015
//CHECK: undefined identifier 'undefinedTwo'.
}
