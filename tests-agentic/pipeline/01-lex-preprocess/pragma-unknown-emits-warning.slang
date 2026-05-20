//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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

void main() {}
