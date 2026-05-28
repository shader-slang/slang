//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/architecture/overview.md#compilation-request-lifecycle
//META: doc_section_digest=4dae689ab71645e4934f3d5e29afb21da2d6bcdf308ea84e96e25981a6c6dd7b
//META: purpose=An `EntryPointRequest` bundles a function name with a pipeline stage; `-stage vertex` produces vertex-shader emit and the stage-specific outputs are visible.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-stage vertex` causes Slang to emit code recognised by
// the downstream toolchain as a vertex shader. In GLSL, that means
// writing through `gl_Position`; in HLSL it means the `SV_Position`
// semantic survives onto the entry function's return. If `-stage` did
// not flow into the `EntryPointRequest` as the doc states, neither
// stage-specific observable would appear.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry vsMain -stage vertex
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry vsMain -stage vertex

struct VOut
{
    float4 pos : SV_Position;
};

VOut vsMain(uint vid : SV_VertexID)
{
    //HLSL: SV_Position
    //GLSL: gl_Position
    VOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    return o;
}
