//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#errortype-and-bottomtype
//META: doc_section_digest=3d0b98b5a9dfdb21e5d846e5291fce8a427595ccbfe5e2bd998b9b125a792d94
//META: purpose=ErrorType is the type of an expression that failed to check and keeps downstream checking from cascading, so a single root cause yields a single diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-cascade property: one undefined identifier poisons
// the initialiser, and the three later expressions that consume the
// poisoned local must not each report a fresh error. CHECK pins the
// single undefined-identifier diagnostic; any cascading mismatch on the
// following lines would leave unmatched diagnostics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = bogusName;
//CHECK:    ^^^^^^^^^ undefined identifier 'bogusName'.
    int y = x + 1;
    int z = x * 2;
    int w = y + z;
    printf("%d\n", w);
}
