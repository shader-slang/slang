//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=b0a9ef0c603cd408e940f93b864d2b441f72e4bf4f04f967c2f2f359024af70e
//META: purpose=`VarDecl` accepts the C-style form `Type VarDeclarator (',' VarDeclarator)* ';'` with an `Initializer`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the C-style VarDecl production with multiple declarators in
// a single statement, each with its own `'=' Expr` initializer.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 2, b = 3;
    // CHECK: a=2,b=3
    printf("a=%d,b=%d\n", a, b);
}
