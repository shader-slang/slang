//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for int8_t carries its scalar built-in across the full [-128, 127] range; MIN and MAX literals round-trip through the type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the int8_t BasicExpressionType leaf: both the
// documented MIN (-128) and MAX (127) values are storable in the
// type and survive arithmetic through int promotion at printf.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int8_t lo = int8_t(-128);
    int8_t hi = int8_t(127);
    // CHECK: lo=-128
    printf("lo=%d\n", int(lo));
    // CHECK: hi=127
    printf("hi=%d\n", int(hi));
}
