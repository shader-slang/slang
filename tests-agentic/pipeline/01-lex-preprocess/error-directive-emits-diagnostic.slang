//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
//META: purpose=The standard C/HLSL `#error` directive is supported and causes the preprocessor to emit a diagnostic with the directive's message text.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `#error MSG` triggers a preprocessor diagnostic
// whose text contains the message body. We surround the `#error`
// with `#if 1` so the diagnostic is reached, and FileCheck the
// emitted diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#if 1
#error This is the error message!
/*CHECK:
 ^^^^^ preprocessor error
 ^^^^^ #error: This is the error message!
*/
#endif

void main() {}
