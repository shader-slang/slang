//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#comments
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=An unterminated block comment is an error per the doc: "It is an error if a block comment that begins in a source unit is not terminated in that source unit."
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc explicitly classifies an unterminated block comment as
// an error. Verify the compiler emits a diagnostic. The actual
// diagnostic Slang fires is `E20001 — unexpected end of file`
// rather than a comment-specific "unterminated block comment"
// message; the spec's claim is satisfied (the input is rejected)
// but the diagnostic does not name the comment. Recorded in the
// bundle README as a doc-vs-compiler mismatch worth tightening
// in either direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 0;
    /* this comment is never closed
}
//CHECK: E20001
