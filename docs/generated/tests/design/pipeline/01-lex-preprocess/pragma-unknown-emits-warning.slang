//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
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
// Deliberately NOT asserting E15101 here. `#pragma <unknown>` reaches
// findPragmaDirective, which falls back to kUnknownPragmaDirective; its
// callback emits only UnknownPragmaDirectiveIgnored (W15601) and then
// SkipToEndOfLine. E15101 (unknown preprocessor directive) is raised for an
// unrecognized `#`-directive name and is unreachable from pragma handling.

void main() {}
