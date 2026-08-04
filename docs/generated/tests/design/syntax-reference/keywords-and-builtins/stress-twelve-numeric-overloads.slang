//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Stress: twelve `kind` overloads — one for each documented core-module scalar (`int8_t`, `int16_t`, `int`, `int64_t`, `uint8_t`, `uint16_t`, `uint`, `uint64_t`, `half`, `float`, `double`, `bool`) — all dispatch distinctly under overload resolution.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Extends `builtin-numeric-types-overload.slang` from four overloads
// to twelve. Each documented scalar type must remain distinguishable
// by overload resolution; any collapse would shift the printed
// sequence away from the expected `1 2 3 4 5 6 7 8 9 10 11 12`.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int8_t v)   { return 1; }
int kind(int16_t v)  { return 2; }
int kind(int v)      { return 3; }
int kind(int64_t v)  { return 4; }
int kind(uint8_t v)  { return 5; }
int kind(uint16_t v) { return 6; }
int kind(uint v)     { return 7; }
int kind(uint64_t v) { return 8; }
int kind(half v)     { return 9; }
int kind(float v)    { return 10; }
int kind(double v)   { return 11; }
int kind(bool v)     { return 12; }

void main()
{
    int8_t a8 = -1;
    int16_t a16 = -1;
    int a32 = -1;
    int64_t a64 = -1;
    uint8_t u8 = 1u;
    uint16_t u16 = 1u;
    uint u32 = 1u;
    uint64_t u64 = 1ULL;
    half h = half(1.0);
    float f = 1.0f;
    double d = 1.0;
    bool b = true;
    // CHECK: 1 2 3 4 5 6 7 8 9 10 11 12
    printf("%d %d %d %d %d %d %d %d %d %d %d %d\n",
        kind(a8), kind(a16), kind(a32), kind(a64),
        kind(u8), kind(u16), kind(u32), kind(u64),
        kind(h), kind(f), kind(d), kind(b));
}
