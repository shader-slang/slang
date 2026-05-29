//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - a hexadecimal floating-point literal (`0x1.0p4`) is a valid FloatingPointLiteral and decodes to its base-16 mantissa times 2^exponent.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: hex-float form `0x1.0p4` has mantissa 1.0 (hex)
// and binary exponent 4, so it equals 16.0. The lexer must accept
// `p`/`P` as the binary-exponent marker.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float v = 0x1.0p4;
    // CHECK: v=16
    printf("v=%d\n", int(v));
}
