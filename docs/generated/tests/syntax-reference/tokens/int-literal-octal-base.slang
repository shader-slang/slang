//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - an octal integer literal (`0777`) lexes as IntegerLiteral and decodes to its base-8 value.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: a leading `0` followed by octal digits is part of
// the IntegerLiteral token; `0777` should decode to decimal 511.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 0777;
    // CHECK: v=511
    printf("v=%d\n", v);
}
