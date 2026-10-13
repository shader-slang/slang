//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=34ae2014df6f94e86e6f1c0103e2b1bc5da580d48dd7ba67686044b7b740ee79
//META: purpose=`LetDecl` parses as `'let' IDENT (':' Type)? ('=' ArgExpr)? ';'`, binding a value without a type annotation.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the LetDecl production with the optional type annotation
// elided. The bound value is observed via printf.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let x = 13;
    // CHECK: x=13
    printf("x=%d\n", x);
}
