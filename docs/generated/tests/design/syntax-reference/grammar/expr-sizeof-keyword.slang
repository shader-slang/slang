//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=18781d96c73775d674cac19ebc4add31667c41ea657c3ad84686dcc6190a3a49
//META: purpose=`KeywordExpr` accepts `'sizeof' '(' Type ')'`.
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
