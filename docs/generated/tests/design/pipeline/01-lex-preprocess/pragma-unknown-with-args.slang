//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=c3ad01b7a04e24b43f66e2d017e85a491e5a0fbbf99a28a1c828245742a23613
//META: purpose=Boundary probe of `#pragma`: an unknown sub-directive followed by argument tokens still produces the "unknown pragma ignored" warning, and the trailing tokens are skipped to end-of-line.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#pragma` sub-directive with argument tokens. The lookup ignores
// the trailing tokens; the warning identifies only the
// sub-directive name. Compilation continues, so the body still
// compiles.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#pragma unknown_pragma_with_args(arg1, arg2, 42)
//CHECK:^^^^^^^^^^^^^^^^^^^^^^^^ unknown pragma ignored
//CHECK:^^^^^^^^^^^^^^^^^^^^^^^^ ignoring unknown directive '#pragma unknown_pragma_with_args'
// CHECK: W15601
// CHECK: E15101

void main() {}
