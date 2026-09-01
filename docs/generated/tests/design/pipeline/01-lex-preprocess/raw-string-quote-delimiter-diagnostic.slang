//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Negative probe of the purely syntactic scan-time checks: a `"` inside a raw string literal's delimiter is rejected while scanning (E10010).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the raw-string arm of literal scanning. The raw-string
// scanner needs the text between `R"` and `(` to be a delimiter it can
// search for again as `)delimiter"`; a `"` in that run would make the
// closing sequence ambiguous, so it is rejected before any value
// decoding happens. E10010 is pinned so the check cannot be renumbered
// away.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const String bad = R"a"b(x)a"b";
//CHECK:                     ^^^^^^^ '"' encountered before '(' in raw string literal. '"' cannot be a part of a delimiter.
// CHECK: E10010

void main() {}
