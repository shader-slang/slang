//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=f0771e16a4cd3fa366077436469b559fb6852160c25c69aae5f00e8413738f6b
//META: purpose=A malformed `operator <garbage>` name is reported once as E20008 and is not flagged a second time by the operator-name-on-non-function rule.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented non-duplication. `operator@` is both malformed as an
// operator spelling and used as a variable name, so a naive implementation
// would emit E20008 for the bad spelling and E20020 for the non-function use.
// Exhaustive mode makes the absence of the second diagnostic assertable: only
// E20008 is annotated, so a second report on this declaration would fail the
// test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int operator@ = 3;
//CHECK:    ^ error E20008
//CHECK:    ^ invalid operator '@'.

void main()
{
}
