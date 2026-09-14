//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=f5c669bb9740dfa642523af3da2be0ac7b679950ab4f326668ed87a90288d1e6
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
