//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=18781d96c73775d674cac19ebc4add31667c41ea657c3ad84686dcc6190a3a49
//META: purpose=A parenthesized expression atom requires a matching `)`; a missing close-paren is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `( Expr )` requires the closing paren. Omitting it
// causes a parse-stage diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = (1 + 2;
    //CHECK: error
    //CHECK: E20001
    printf("x=%d\n", x);
}
