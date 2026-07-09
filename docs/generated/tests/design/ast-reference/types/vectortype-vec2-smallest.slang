//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A VectorExpressionType at the smallest documented arity (count 2) carries both components; an int2 reads back its two elements.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the smallest vector arity. An int2 carries a
// count-2 VectorExpressionType; both components are read via .x/.y.
// CHECK asserts the two values, exercising the low edge of the count.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int2 v = int2(-3, 8);
    // CHECK: v=-3,8
    printf("v=%d,%d\n", v.x, v.y);
}
