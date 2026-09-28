//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=34ae2014df6f94e86e6f1c0103e2b1bc5da580d48dd7ba67686044b7b740ee79
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
