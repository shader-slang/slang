//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType at the largest standard dimension (N=4) supports a full-length swizzle (.wzyx) that permutes all components.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the large end of vector dim: a `float4`
// VectorExpressionType supports a length-4 swizzle that addresses
// each component in a permuted order. Exercises the count=4 case
// of the (element-type, count) operand pair.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(1.0, 2.0, 3.0, 4.0);
    float4 w = v.wzyx;
    // CHECK: w=4,3,2,1
    printf("w=%d,%d,%d,%d\n", int(w.x), int(w.y), int(w.z), int(w.w));
}
