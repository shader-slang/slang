//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=f52a7151fb266a0395ad9852aa535a328de8a8e0c05ff15ae8b0008b93f3fcaf
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
