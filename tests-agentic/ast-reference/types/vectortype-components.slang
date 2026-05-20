//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A VectorExpressionType is parameterised by element-type and count; a floatN value's N components are independently readable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `VectorExpressionType` carries `(element-type, count)` in its
// operand list. The user-facing consequence: a `float4` value has
// four float components addressable as `.x .y .z .w` (and the
// numeric values survive evaluation).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(10.0, 20.0, 30.0, 40.0);
    // CHECK: v=10,20,30,40
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
