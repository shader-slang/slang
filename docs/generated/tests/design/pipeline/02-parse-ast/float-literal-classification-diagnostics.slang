//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=f0771e16a4cd3fa366077436469b559fb6852160c25c69aae5f00e8413738f6b
//META: purpose=A floating-point literal whose classification is a bad significand or a bad suffix is diagnosed at parse time, when the token text is turned into a value.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two error arms of the literal classification. `1e` scans as a
// single floating-point token — the lexer accepted it — but has no exponent
// digits, so decoding it is what fails; `1.5q` scans fine too but carries a
// suffix that maps to no floating-point type. Both diagnostics name the
// offending text back, and each caret spans the whole literal token, which is
// the evidence that the report comes from the literal expression rather than
// from token scanning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const float badSignificand = 1e;
//CHECK:                            ^^ error E39999
//CHECK:                            ^^ invalid floating-point number '1e' on floating-point literal

static const float badSuffix = 1.5q;
//CHECK:                       ^^^^ error E39999
//CHECK:                       ^^^^ invalid suffix 'q' on floating-point literal

void main()
{
}
