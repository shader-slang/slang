//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=A SwizzleExpr `v.zyx` reorders vector components; every text-emit target preserves the swizzle in a target-appropriate form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SwizzleExpr is one of the few expression-level claims whose
// lowering is observably target-divergent. HLSL/GLSL preserve the
// `.zyx` syntax; SPIR-V emits OpVectorShuffle. We check each
// target's canonical form. The vector source comes from a uniform
// so DCE cannot constant-fold it away.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

uniform float3 v;
RWStructuredBuffer<float3> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = v.zyx;
    // HLSL: .zyx
    // GLSL: .zyx
    // SPV: OpVectorShuffle
}
