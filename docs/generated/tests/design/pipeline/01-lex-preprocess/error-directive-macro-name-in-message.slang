//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
//META: purpose=Boundary probe of `#error`: an identifier in the message body is preserved verbatim, observable in the emitted diagnostic.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#error` followed by a token that happens to match a defined
// macro name. The observed behavior is that the message is the
// raw source text — the macro name is not expanded. We check that
// the diagnostic carries the verbatim identifier `WHAT_TOKEN`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#define WHAT_TOKEN expanded

#error message-contains WHAT_TOKEN
//CHECK: WHAT_TOKEN
//CHECK: E15900

void main() {}
