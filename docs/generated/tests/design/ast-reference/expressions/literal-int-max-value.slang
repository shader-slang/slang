//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=Verifies that an IntegerLiteralExpr at the documented int MAX (2147483647) parses and round-trips through evaluation as that exact value.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Upper-edge boundary for the signed int base type. The LiteralExpr
// must preserve the value all the way through to the runtime print.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 2147483647;
    // CHECK: v=2147483647
    printf("v=%d\n", v);
}
