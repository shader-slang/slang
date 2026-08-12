//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=4608df68ba69a62121bc1827dea7e4379ab2e4bc223340d0aead0d9cc6654fc2
//META: purpose=Boundary probe of `#pragma`: an unknown sub-directive name is ignored with a warning diagnostic, rather than rejected as a hard error.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An unknown `#pragma` sub-directive (not `once`, not `warning`) is
// looked up in the pragma callback table, found absent, and treated
// as a warning ("unknown pragma ignored"). Compilation continues.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#pragma totally_unknown
//CHECK:^^^^^^^^^^^^^^^ unknown pragma ignored
//CHECK:^^^^^^^^^^^^^^^ ignoring unknown directive '#pragma totally_unknown'
// CHECK: W15601
// CHECK: E15101

void main() {}
