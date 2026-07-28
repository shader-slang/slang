//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#errortype-and-bottomtype
//META: doc_section_digest=6ecef9bba301e535814d4c79a05d988bd802f1af0013f77d77ba877cec59276f
//META: purpose=ErrorType is the type of a failed expression; downstream uses of the failed value do not produce cascading type-mismatch diagnostics, so a single root cause yields a single diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies ErrorType's non-cascade claim: a single undefined
// identifier makes the sub-expression an ErrorType, and the three
// downstream arithmetic uses of that value must NOT each emit a new
// error. CHECK matches the single undefined-identifier diagnostic;
// non-exhaustive absorbs the paired span form only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = bogusName;
//CHECK:    ^^^^^^^^^ undefined identifier 'bogusName'.
    int y = x + 1;
    int z = x * 2;
    int w = y + z;
}
