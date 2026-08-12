//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=6d196fa35abfeab93be9899c03adbb528a464f03a35cc28a6454968f1c88eb2a
//META: purpose=The `u` suffix on an integer literal makes the token's type unsigned; the suffix is part of the literal token's raw text and is decoded by the parser/checker.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `42u` is typed as unsigned. We feed the literal into a
// function overload that accepts only `uint`; if the suffix were
// dropped or interpreted as a different type, overload resolution
// would fail to find a unique match and the test would not compile.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }

void main()
{
    // CHECK: kind=2
    printf("kind=%d\n", kind(42u));
}
