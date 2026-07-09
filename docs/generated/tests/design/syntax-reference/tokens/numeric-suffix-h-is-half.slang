//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=0e1e5b6a05be01d6e2a0fbe1d41469c918569522adb5871ac6efa8d2a0811ba4
//META: purpose=Boundary - the `h` suffix on a FloatingPointLiteral types it as `half` (16-bit float).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the `h` suffix is documented alongside `f` as a
// float-literal suffix. `3.5h` should resolve to `half`, exercised
// through an overload set that distinguishes `float`, `double`, and
// `half`.

//TEST:INTERPRET(filecheck=CHECK):

int kind(float v) { return 1; }
int kind(double v) { return 2; }
int kind(half v) { return 3; }

void main()
{
    // CHECK: kind=3
    printf("kind=%d\n", kind(3.5h));
}
