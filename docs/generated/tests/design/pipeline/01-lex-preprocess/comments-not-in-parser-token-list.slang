//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#what-the-lexer-does-not-do
//META: doc_section_digest=effeedc2dec5b1cd62b793dca8f844925076b3dea4a96212a0cd91d16b3e59d4
//META: purpose=The lexer emits whitespace and comments as their own token types but the preprocessor's token collection filters them, so the parser never sees a comment embedded mid-expression.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the filtering half of the "the lexer does not skip
// whitespace or comments" claim, at the one place a leftover comment
// token would be impossible to ignore: inside a macro body, where the
// stored token sequence is replayed straight into the expression the
// parser reads. A block comment sits between the two operands of `+`
// and a line comment ends the definition; if either survived into the
// replayed token list the expansion would not parse. Printing 3 proves
// the surviving tokens were exactly `1 + 2`.

//TEST:INTERPRET(filecheck=CHECK):

#define SUM 1 /* inline block comment */ + 2 // trailing line comment

void main()
{
    // CHECK: sum=3
    printf("sum=%d\n", SUM);
}
