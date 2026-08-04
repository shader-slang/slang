//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#type-expression-family-pointertypeexpr-functypeexpr-tupletypeexpr-andtypeexpr-modifiedtypeexpr-thistypeexpr
//META: doc_section_digest=4254abda0523877f7fb67acc9da1ad7de3884b6d869e71f92f12575587230f39
//META: purpose=A ModifiedTypeExpr `const T` carries a modifier prefix on the type; reads of the const-qualified value return the initialized value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists ModifiedTypeExpr as the AST kind for "type expression
// with modifier prefixes". `const int` is the canonical example: the
// const modifier attaches to the type via a ModifiedTypeExpr.

//TEST:INTERPRET(filecheck=CHECK):

static const int kK = 42;

void main()
{
    // CHECK: k=42
    printf("k=%d\n", kK);
}
