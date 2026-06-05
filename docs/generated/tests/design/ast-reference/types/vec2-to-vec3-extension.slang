//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A float2 can be extended to a float3 via the explicit constructor float3(v2, w); element-type is preserved and the new lane carries the supplied scalar.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: dimension-extension at the vector boundary. The
// canonical surface is explicit construction `float3(v2, w)`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2 v2 = float2(10.0, 20.0);
    float3 v3 = float3(v2, 30.0);
    // CHECK: v3=10,20,30
    printf("v3=%d,%d,%d\n", int(v3.x), int(v3.y), int(v3.z));
}
