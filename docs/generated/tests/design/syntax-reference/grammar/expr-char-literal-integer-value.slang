//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=c7d6780d315f67918c23ddaaf2cd151fb568fe783747ef75230b9c640ed1629f
//META: purpose=`CHAR_LIT` is a `Literal` that lowers to an `IntegerLiteralExpr` rather than a dedicated character node.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a character literal carries an integer value: 'A'
// converted with a constructor-style cast plus 1 prints 66, i.e. the
// code point 65 participated directly in integer arithmetic.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: v=66
    printf("v=%d\n", int('A') + 1);
}
