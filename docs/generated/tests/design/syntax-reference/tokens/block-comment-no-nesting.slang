//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=d3d4c02ac4dafd74393b443c556304f894229442554014fb013287b9cae88960
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
