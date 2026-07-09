//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00+00:00
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=b26a0318742c927611640f41150c8e4198d2c7febf7724b24a1dc8fc4cac2242
//META: purpose=An SV_DepthLessEqual fragment output is recorded as the FragDepthLess system-value kind and the SPIR-V emit carries the conservative-depth DepthLess execution mode on the gl_FragDepth (BuiltIn FragDepth) output.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the FragDepthLess companion of the conservative-depth pair:
// an SV_DepthLessEqual output funnels through the same
// legalizeEntryPointsForGLSL refinement, and the SPIR-V emitter derives
// the DepthLess execution mode. CHECKs pin the DepthLess
// OpExecutionMode and the BuiltIn FragDepth decoration on the output.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float main(float4 pos : SV_Position) : SV_DepthLessEqual
{
    // Depth depends on a varying input so the output is not folded away.
    return pos.z * 0.5 - 0.25;
}

// CHECK-DAG: OpEntryPoint Fragment
// CHECK-DAG: OpExecutionMode {{%[a-zA-Z_0-9]+}} DepthLess
// CHECK-DAG: BuiltIn FragDepth
