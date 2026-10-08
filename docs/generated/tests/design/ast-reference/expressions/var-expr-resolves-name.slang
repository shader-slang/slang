//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#varexpr-and-declrefexpr
//META: doc_section_digest=d41eb7e3da9d1c66fee3952dce01afeadec7647fa5526911668777fd6602e274
//META: purpose=A bare identifier in expression context parses as a VarExpr; after lookup it resolves to the declared variable and reads its value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The expression `g` in `printf("...%d", g)` is a VarExpr that the
// checker resolves to the global declaration named `g`. The printed
// value confirms the reference resolved to the right declaration.

//TEST:INTERPRET(filecheck=CHECK):

static const int g = 42;

void main()
{
    // CHECK: g=42
    printf("g=%d\n", g);
}
