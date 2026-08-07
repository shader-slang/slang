//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=7aa90a50fa61e43bbfc5d0af2203fd4cee827be1fbe30b61a2022eca4801f55f
//META: purpose=Verifies the SPIR-V varying-param legalizer splits a struct input/output into per-field OpVariable globals decorated with sequential Location indices, routing SV_Position to BuiltIn Position.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the SPIR-V arm of varying-parameter legalization (the same struct
// shape as the GLSL flatten test, but observed in SPIR-V assembly). Each
// VIn/VOut field becomes its own Input/Output OpVariable; the legalizer
// emits `OpDecorate %v Location N` with locations counted per
// in-direction and out-direction, and routes the SV_Position field to
// `OpDecorate %gl_Position BuiltIn Position`. CHECK pins those
// decorations.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage vertex

struct VIn { float3 pos; float2 uv; float4 color; }
struct VOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : COLOR0; }

[shader("vertex")]
VOut main(VIn input)
{
    VOut o;
    o.pos = float4(input.pos, 1.0);
    o.uv = input.uv;
    o.color = input.color;
    return o;
}

// Input fields decorate with sequential input locations.
//SPV-DAG: OpDecorate %input_pos Location 0
//SPV-DAG: OpDecorate %input_uv Location 1
//SPV-DAG: OpDecorate %input_color Location 2
// SV_Position routes to the Position builtin, not a user location slot.
//SPV-DAG: OpDecorate %gl_Position BuiltIn Position
// User-semantic outputs decorate with their own location numbering.
//SPV-DAG: OpDecorate %entryPointParam_main_uv Location 0
//SPV-DAG: OpDecorate %entryPointParam_main_color Location 1
