//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=The legacy MSVC infinity form with a leading zero (`0#INF`) lexes as a single FloatingPointLiteral rather than an integer followed by `#INF`.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `0#INF` is consumed as one floating-point token: the
// `#` never reaches the preprocessor as a Pound token and `INF` is not
// a stray identifier, either of which would fail compilation. The
// overload probe returning kind=1 proves the token both lexes as a
// unit and decodes as a floating-point value. (The doc does not state
// the decoded numeric value, so this test deliberately asserts only
// the token classification, not infinity-ness.)

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x) { return 1; }
int probe(int x) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", probe(0#INF));
}
