//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Check-level recovery is "continue with a placeholder type" so one error does not cascade; an undefined identifier in one statement does not erase the diagnostic for an unrelated error in the next.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Failure modes section states that recovery uses placeholder
// types so subsequent statements still produce their own errors.
// Verify: both an undefined-identifier and a separate stray-break
// diagnostic are emitted (rather than the first one silencing the
// second).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int y = unknownX;
//CHECK:    ^^^^^^^^ undefined identifier
    break;
/*CHECK:
    ^^^^^ 'break' must appear inside loop or switch constructs
*/
// CHECK: E30015
// CHECK: E30003
}
