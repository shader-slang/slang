//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Hexadecimal prefix is case-insensitive: `0X` (uppercase) parses identically to `0x` (lowercase).
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's `HexIntegerLiteralBody` production explicitly lists
// `('0x' | '0X')` as the prefix alternatives. Verify the uppercase
// prefix accepts a hex literal that equals its lowercase counterpart.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int lower = 0xff;
    int upper = 0XFF;
    //CHECK: lower=255 upper=255 eq=1
    printf("lower=%d upper=%d eq=%d\n", lower, upper, lower == upper ? 1 : 0);
}
