//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A VectorExpressionType is parameterised by element-type and count; a float4 value's four components are independently readable by swizzle.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies VectorExpressionType carries (element-type, count): a
// float4 value has four float components addressable as .x .y .z .w.
// Each component is printed; CHECK asserts the four distinct values,
// proving the count-4 vector shape and per-component read.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(10.0, 20.0, 30.0, 40.0);
    // CHECK: v=10,20,30,40
    printf("v=%d,%d,%d,%d\n", int(v.x), int(v.y), int(v.z), int(v.w));
}
