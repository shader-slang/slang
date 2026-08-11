//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=1e8f144399f6073f93949619c72c2230cfb4349def0ab49ac5088e68f85aa4ab
//META: purpose=Verifies that an IntegerLiteralExpr at the documented int MIN (-2147483648) parses and round-trips through evaluation as that exact value.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for the int base type. The LiteralExpr family's
// `value: IntegerLiteralValue` must carry the full negative range.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = -2147483648;
    // CHECK: v=-2147483648
    printf("v=%d\n", v);
}
