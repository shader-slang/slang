//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
//META: purpose=Negative probe of the purely syntactic scan-time checks: a digit outside the literal's base is rejected while scanning (E10003).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the base-checking arm of literal scanning: the scanner has
// to find where a literal ends, and while walking the digit run it
// rejects a digit the base cannot hold. `0b102` names base 2 but
// contains a `2`, so the diagnostic fires on that one character
// rather than on the whole token, and it is pinned to E10003 with the
// offending digit and base in the message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const int bad = 0b102;
//CHECK:                   ^ invalid digit for base-2 literal: '2'
// CHECK: E10003

void main() {}
