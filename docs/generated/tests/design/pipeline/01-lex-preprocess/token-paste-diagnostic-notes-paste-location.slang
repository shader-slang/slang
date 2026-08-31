//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=d51763c65e99c32e7704e3dec4e69c5776526cb88e95bd9188137ae60ad362b9
//META: purpose=A pasted token is re-lexed from a fresh token-paste source view, and a diagnostic raised on it is reported against that view plus a note pointing back at the `##` that initiated the paste.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the constructed-token rule for `##`: the
// paste result is lexed afresh out of its own source view, and that
// view records the `##` token as its initiating location. Pasting
// `0b1` onto `2` makes `0b12`, which the re-lex rejects as a digit
// outside base 2. The diagnostic must therefore be attributed to the
// synthetic "token paste" file rather than to any line of this file,
// and the initiating-location walk must add the paste-location note.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)

static const int pasted = CAT(0b1, 2);

// CHECK-DAG: token paste
// CHECK-DAG: 10003
// CHECK-DAG: see token pasted location

void main() {}
