//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00+00:00
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=b26a0318742c927611640f41150c8e4198d2c7febf7724b24a1dc8fc4cac2242
//META: purpose=An SV_DepthGreaterEqual fragment output is recorded as the FragDepthGreater system-value kind and the SPIR-V emit carries the conservative-depth DepthGreater execution mode on the gl_FragDepth (BuiltIn FragDepth) output.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that returning a depth value through SV_DepthGreaterEqual
// takes the FragDepthGreater system-value path in
// legalizeEntryPointsForGLSL and that the SPIR-V emitter derives the
// conservative-depth execution mode from it. The CHECKs pin the
// DepthGreater OpExecutionMode on the fragment entry point and the
// BuiltIn FragDepth decoration on the depth output variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

[shader("fragment")]
float main(float4 pos : SV_Position) : SV_DepthGreaterEqual
{
    // Depth depends on a varying input so the output is not folded away.
    return pos.z * 0.5 + 0.25;
}

// CHECK-DAG: OpEntryPoint Fragment
// CHECK-DAG: OpExecutionMode {{%[a-zA-Z_0-9]+}} DepthGreater
// CHECK-DAG: BuiltIn FragDepth
