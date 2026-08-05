//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=4c9f71a256907b7a79759ca00a313ad651ac24d85015af4f3fe1673fd66d9dde
//META: purpose=Core-module builtins are excluded from the did-you-mean candidate set, so misspelling a builtin produces no suggestion.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the builtin-exclusion rule. `sqr` is one edit from the core-module
// `sqrt`, so an unfiltered candidate walk would suggest it; the exclusion is
// what keeps the suggestion list from being dominated by the core module.
// CHECK pins the undefined-identifier error and its message, and exhaustive
// mode turns "no suggestion note came with it" into part of the assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    float y = sqr(2.0);
//CHECK: E30015
//CHECK: undefined identifier 'sqr'.
}
