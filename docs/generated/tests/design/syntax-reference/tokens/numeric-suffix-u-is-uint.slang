//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
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
