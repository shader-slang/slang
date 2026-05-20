//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
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
