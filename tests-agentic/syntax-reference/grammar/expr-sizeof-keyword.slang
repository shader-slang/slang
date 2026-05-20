//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
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
