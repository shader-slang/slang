//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=BasicExpressionType for uint at MAX (0xFFFFFFFF) + 1 wraps to 0 per unsigned-wrap semantics.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: uint MAX is 0xFFFFFFFF (4294967295). Adding 1 must
// wrap modulo 2^32 back to 0. The BasicExpressionType for `uint`
// preserves the scalar identity through the overflow.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint hi = uint(0xFFFFFFFF);
    uint wrap = hi + 1u;
    // CHECK: hi=4294967295
    printf("hi=%u\n", hi);
    // CHECK: wrap=0
    printf("wrap=%u\n", wrap);
}
