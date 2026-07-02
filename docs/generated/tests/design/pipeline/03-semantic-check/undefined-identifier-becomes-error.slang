//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=An unresolved name produces a diagnostic; the checker recovers by treating the reference as ErrorType so the failure does not cascade.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Failure modes section says unresolved decls become `ErrorType`-typed
// and the checker continues with placeholders. Verify the user-visible
// half: the undefined-identifier diagnostic fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int x = unknownName;
//CHECK:    ^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^ undefined identifier 'unknownName'.
// CHECK: E30015
}
