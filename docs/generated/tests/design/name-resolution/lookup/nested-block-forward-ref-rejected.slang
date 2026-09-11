//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=fff04d2b441a5ad2e4ef387152ddc76216942a0668864eb5cdfb1c9b98e7e532
//META: purpose=Block-local hiding reaches into nested inner blocks, so a use of the name from a nested block textually before the declaration is undefined (E30015).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the hidden-until-declared filter is not limited to the
// statement level of the declaration itself. `m` is declared at the end
// of the function body but referenced from inside a nested block that
// precedes it; with no outer decl of that name the result is empty and
// the caller diagnoses. CHECK annotates the short and interpolated
// forms plus the stable code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    {
        int y = m + 1;
//CHECK:        ^ undefined identifier
//CHECK:        ^ undefined identifier 'm'.
// CHECK: E30015
        printf("y=%d\n", y);
    }
    int m = 5;
}
