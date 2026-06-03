//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - the `ull` suffix on an IntegerLiteral types it as `uint64_t` (unsigned 64-bit).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: documented integer-literal suffixes go beyond the
// single-character forms `u` and `l`. `42ull` should resolve to
// `uint64_t`, exercised through an overload set that distinguishes
// all four integer widths.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }
int kind(int64_t v) { return 3; }
int kind(uint64_t v) { return 4; }

void main()
{
    // CHECK: kind=4
    printf("kind=%d\n", kind(42ull));
}
