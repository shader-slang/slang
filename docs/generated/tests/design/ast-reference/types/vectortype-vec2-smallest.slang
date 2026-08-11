//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A VectorExpressionType at the smallest vector arity (count 2) carries both components; an int2 reads back its two elements.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the low edge of the vector count operand. An int2
// is the smallest vector arity; both components are read back through
// .x/.y and CHECK asserts the two distinct values, so a collapse of the
// two-element shape to a scalar would be visible.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int2 v = int2(-3, 8);
    // CHECK: v=-3,8
    printf("v=%d,%d\n", v.x, v.y);
}
