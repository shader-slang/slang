//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for uint8_t spans [0, 255]; the lower-edge 0 and upper-edge 255 both round-trip through the scalar.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the uint8_t BasicExpressionType: explores the
// lower edge (0) and upper edge (255) of the documented unsigned
// 8-bit range. Both values are observable through int(...) widening
// at printf.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint8_t lo = uint8_t(0);
    uint8_t hi = uint8_t(255);
    // CHECK: lo=0
    printf("lo=%d\n", int(lo));
    // CHECK: hi=255
    printf("hi=%d\n", int(hi));
}
