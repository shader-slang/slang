//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A SwizzleExpr `v.zyx` reorders vector components; every text-emit target preserves the swizzle in a target-appropriate form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a vector component-reorder survives lowering on every
// text-emit target, which is the one expression kind whose emitted form is
// genuinely target-divergent. The source vector comes from a uniform so the
// folder cannot precompute the result, and the reordered value is stored to a
// buffer so DCE cannot drop it. Each target's CHECK asserts that target's
// canonical spelling: the `.zyx` suffix where the target has swizzle syntax,
// `OpVectorShuffle` on SPIR-V, and an explicit per-component rebuild on the
// C-family targets.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform float3 v;
RWStructuredBuffer<float3> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = v.zyx;
    // HLSL: .zyx
    // GLSL: .zyx
    // SPV: OpVectorShuffle
    // METAL: .zyx
    // WGSL: .zyx
    // CUDA: {{.*}}.z, {{.*}}.y, {{.*}}.x
    // CPP: {{.*}}.z, {{.*}}.y, {{.*}}.x
}
