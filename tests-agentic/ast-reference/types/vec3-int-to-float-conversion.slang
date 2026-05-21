//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=Element-type widening conversion at the vector boundary: a VectorExpressionType<int, 3> converts to VectorExpressionType<float, 3> via explicit construction.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: element-type conversion at the vector level. The
// (element-type, count) pair changes element type from int to
// float while the lane count is preserved.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int3 i = int3(1, 2, 3);
    float3 f = float3(i);
    f.x += 0.5f;
    // CHECK: f=1,2,3
    printf("f=%d,%d,%d\n", int(f.x), int(f.y), int(f.z));
}
