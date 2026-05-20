//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:41Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ast-reference/index.md#cross-cutting-topics
//META: doc_section_digest=36f1c960f3b34c6c84e64ac6cb1dc61adf1ed16c98013e2b8374b4a2928c2950
//META: purpose=The index doc states that AST nodes lower to Slang IR which retires most of them; a Slang-level matrix-vector multiply emits as the target's mul-idiom rather than as a Slang-AST-shaped artifact.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The #cross-cutting-topics section quotes pipeline/04-ast-to-ir
// as "how AST nodes lower to Slang IR (which retires most of
// them)". The cross-cutting evidence that AST nodes are retired
// during lowering is that the emitted target text does NOT carry
// the original Slang AST shape -- a Slang-level matrix*vector
// multiply, which is a single InvokeExpr in the AST, lowers
// through IR and emits as the HLSL `mul` intrinsic.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(float(tid.x), 1.0, 2.0);
    float3x3 m = float3x3(1, 0, 0,
                          0, 2, 0,
                          0, 0, 3);
    float3 r = mul(m, v);
    // CHECK: mul(
    outBuf[0] = r.x + r.y + r.z;
}
