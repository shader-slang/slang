//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=e44df04d9dcfdbcd540c6b84f9f1c3b8b556eee2b84931196b66361c17faa2df
//META: purpose=Block comments do not nest; an inner `/*` inside `/* ... */` is content, and the outer comment ends at the first `*/`.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that nested block comments are NOT supported. The outer
// `/* outer /* inner */` closes at the FIRST `*/`. If nesting were
// supported, `static const int x = 7;` would still be inside the
// comment and the printf would fail to resolve `x`. With the
// documented non-nesting behavior, `x` is declared and printable.

//TEST:INTERPRET(filecheck=CHECK):

/* outer /* inner */ static const int x = 7;

void main()
{
    // CHECK: x=7
    printf("x=%d\n", x);
}
