//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
//META: purpose=Negative probe of the purely syntactic scan-time checks: a leading `0` on a decimal-looking literal raises the legacy-octal warning (W10002).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that literal scanning performs the small set of syntactic
// checks the split allows even though value decoding is deferred:
// `017` is scanned as a legacy octal literal and the scanner warns
// about the `0` prefix. The caret must span the whole literal token
// the scanner recorded, and the warning is pinned to W10002 so a
// renumbering cannot pass silently.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

static const int legacy = 017;
//CHECK:                  ^^^ '0' prefix indicates octal literal
// CHECK: W10002

void main() {}
