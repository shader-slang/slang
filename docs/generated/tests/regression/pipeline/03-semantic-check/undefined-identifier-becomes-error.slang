//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
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
