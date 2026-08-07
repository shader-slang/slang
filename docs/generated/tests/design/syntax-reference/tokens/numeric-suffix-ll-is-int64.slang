//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=0e1e5b6a05be01d6e2a0fbe1d41469c918569522adb5871ac6efa8d2a0811ba4
//META: purpose=Boundary - the `ll` suffix on an IntegerLiteral types it as `int64_t` (signed 64-bit).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: `42ll` should resolve to `int64_t`. Distinguished
// from `int` and `uint64_t` via overload set so the suffix decoding
// is unambiguously checked.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }
int kind(int64_t v) { return 3; }
int kind(uint64_t v) { return 4; }

void main()
{
    // CHECK: kind=3
    printf("kind=%d\n", kind(42ll));
}
