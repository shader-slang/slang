//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=0e1e5b6a05be01d6e2a0fbe1d41469c918569522adb5871ac6efa8d2a0811ba4
//META: purpose=Boundary - a FloatingPointLiteral with a leading dot and no integer part (`.5`) is a single token starting at the `.`.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: `.5` is a valid float literal. If the lexer
// dispatched `.` as the `Dot` token first, the following `5` would
// be a separate integer literal and the expression would not type-
// check as a single float.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float v = .5;
    int doubled = int(v * 2.0f);
    // CHECK: doubled=1
    printf("doubled=%d\n", doubled);
}
