//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
//META: purpose=Verifies a fragment SV_Position input maps to gl_FragCoord and SV_IsFrontFace maps to the boolean gl_FrontFacing builtin in emitted GLSL.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the fragment-stage input system-value mapping: SV_Position as a
// fragment *input* (not the vertex output gl_Position) maps to
// gl_FragCoord, and the boolean SV_IsFrontFace maps to gl_FrontFacing.
// Both values feed the returned color so neither is eliminated; CHECK
// asserts both builtin names appear.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment

[shader("fragment")]
float4 main(float4 pos : SV_Position, bool ff : SV_IsFrontFace) : SV_Target
{
    return pos + (ff ? float4(1) : float4(0));
}

// Fragment SV_Position is the screen-space coordinate builtin.
//GLSL-DAG: gl_FragCoord
// SV_IsFrontFace is the boolean facing builtin.
//GLSL-DAG: gl_FrontFacing
