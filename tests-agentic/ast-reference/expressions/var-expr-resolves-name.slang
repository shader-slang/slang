//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#varexpr-and-declrefexpr
//META: doc_section_digest=1048119061754018682d979f5542202272f2f067f0a41b1b806037278d497506
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
