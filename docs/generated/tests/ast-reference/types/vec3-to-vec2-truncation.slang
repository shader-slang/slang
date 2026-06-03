//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=d7566fda8bc9eff74409576c78889c2b13b22cd59e2676d755123b9475f668d4
//META: purpose=A float3 can be projected to a float2 via the .xy swizzle; the dimension N reduces from 3 to 2 with element-type preserved.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: dimension-reduction at the vector boundary. Swizzle
// .xy yields a length-2 vector from a length-3 vector, sharing
// element type with the source.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float3 v3 = float3(10.0, 20.0, 30.0);
    float2 v2 = v3.xy;
    // CHECK: v2=10,20
    printf("v2=%d,%d\n", int(v2.x), int(v2.y));
}
