//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=b0a9ef0c603cd408e940f93b864d2b441f72e4bf4f04f967c2f2f359024af70e
//META: purpose=`VarDeclarator` accepts an `ArraySuffix` of the form `'[' Expr ']'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a C-style VarDecl declarator parses an ArraySuffix
// `[N]`. The element is then read via the subscript postfix.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int xs[3] = { 10, 20, 30 };
    // CHECK: mid=20
    printf("mid=%d\n", xs[1]);
}
