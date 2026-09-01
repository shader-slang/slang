//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=4e9de34fbc8a8877c6b73c6110db4d1ebef2d0ddd10da2bdfd2742413c1d6015
//META: purpose=`KeywordExpr` accepts `'sizeof' '(' ArgExpr ')'`, where a type name parses as the operand expression.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `sizeof(Type)` keyword-expression production. The
// surface accepts a parenthesized type. The numeric value is not
// the production claim; the production claim is that the syntax
// parses and yields a usable integer.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = int(sizeof(int));
    // CHECK: nonzero=1
    printf("nonzero=%d\n", s > 0 ? 1 : 0);
}
