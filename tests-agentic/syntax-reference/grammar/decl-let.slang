//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=abe6aa9c9154b51a19afb3ed32690f7bbf63d03b221691bbff010e946951f92c
//META: purpose=`LetDecl` parses as `'let' IDENT (':' Type)? '=' Expr ';'` with a mandatory initializer.
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
