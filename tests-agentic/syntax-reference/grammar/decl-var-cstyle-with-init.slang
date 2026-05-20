//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=abe6aa9c9154b51a19afb3ed32690f7bbf63d03b221691bbff010e946951f92c
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
