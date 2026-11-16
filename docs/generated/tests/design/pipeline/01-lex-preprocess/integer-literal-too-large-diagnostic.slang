//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=7e4fe030e4c8d273dce73bfc62475650108e02955445a796df82003091653dfa
//META: purpose=Negative probe of decode-time integer errors: a literal too large for any integer type is reported by the integer value-extraction helper (E10012).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the scanner accepts an arbitrarily long digit run as
// one literal token and that the range failure surfaces later, when
// the value is actually extracted. The literal below has 23 digits, so
// no integer type can hold it; the decode-time diagnostic is pinned to
// E10012, and the caret must span the whole digit run so a literal
// that merely overflowed one narrower type would not match.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const int huge = 99999999999999999999999;
//CHECK:                ^^^^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type
// CHECK: E10012

void main() {}
