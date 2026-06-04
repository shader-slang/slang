//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=VectorExpressionType supports element-type conversion via explicit construction from a vector with a different element type; the resulting components have the target's element type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the mixed-arithmetic axis: a `float3` constructed
// from an `int3` produces a `float3` (element-type widening). The
// resulting vector adds elementwise with another `float3`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3 a = float3(1.0, 2.0, 3.0);
    float3 b = float3(int3(10, 20, 30));
    float3 c = a + b;
    // CHECK: c=11,22,33
    printf("c=%d,%d,%d\n", int(c.x), int(c.y), int(c.z));
}
