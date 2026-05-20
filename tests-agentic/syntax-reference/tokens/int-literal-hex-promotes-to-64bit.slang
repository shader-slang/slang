//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - a hex IntegerLiteral that does not fit in 32 bits (`0x100000000`) is still a valid token and resolves to `int64_t`.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: `0x100000000` is one past `uint32` MAX. The lexer
// must accept the whole token; later stages must select a 64-bit
// type so the value can be represented. The overload set discriminates
// `int` from `int64_t` to confirm the chosen type.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int v) { return 1; }
int kind(uint v) { return 2; }
int kind(int64_t v) { return 3; }
int kind(uint64_t v) { return 4; }

void main()
{
    // CHECK: kind=3
    printf("kind=%d\n", kind(0x100000000));
}
