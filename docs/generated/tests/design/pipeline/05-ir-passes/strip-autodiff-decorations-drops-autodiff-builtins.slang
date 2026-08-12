//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#differentiation-autodiff
//META: doc_section_digest=ea401b301b3b30f1b1ab3d45dcb7d2edbdb15bc388703f42aefd38326f090fb4
//META: purpose=On a module with no autodiff constructs the core-module autodiff builtins lose their keep-alive decorations and are dead-code-eliminated, so no differential-pair machinery reaches the emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable consequence of `stripAutoDiffDecorations`,
// which the autodiff driver row says runs in place of the full autodiff
// finalization on modules that contain no differentiation. This shader
// declares nothing `[Differentiable]`; the CHECKs assert the entry point
// is emitted while no `DifferentialPair` type and no forward / reverse
// derivative wrapper symbol survives, which is only true once the
// keep-alive decorations were stripped and DCE removed the builtins.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float v = float(tid.x);
    buf[0] = v * v;
}

// HLSL: void main
// HLSL-NOT: DifferentialPair
// HLSL-NOT: s_fwd_
// HLSL-NOT: s_bwd_

// SPIRV: OpEntryPoint GLCompute
// SPIRV-NOT: DifferentialPair
// SPIRV-NOT: s_fwd_
// SPIRV-NOT: s_bwd_
