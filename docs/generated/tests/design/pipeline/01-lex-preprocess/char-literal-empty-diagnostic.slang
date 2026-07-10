//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=3d1fe0ccb147581731904561116b628f50946cd1ba50b6e57e6f7814154570a2
//META: purpose=Negative probe: an empty character literal is diagnosed at scan time as an illegal character literal (E10001).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that literal scanning validates structure even though it
// defers value decoding: a character literal with no character is
// rejected during the scan with the illegal-character-literal
// diagnostic, pinned to code E10001.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    uint c = '';
//CHECK:     ^^ illegal character literal
// CHECK: E10001
}
