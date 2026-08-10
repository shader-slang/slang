//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=7aa90a50fa61e43bbfc5d0af2203fd4cee827be1fbe30b61a2022eca4801f55f
//META: purpose=Verifies a vertex entry point's struct-typed varying input and output are flattened into per-field GLSL global in/out variables with sequential location indices.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the struct-flattening arm of the GLSL entry-point legalizer. A
// `VIn` input struct (three user fields) and a `VOut` output struct (one
// SV_Position + two user-semantic fields) each get decomposed into
// separate `in`/`out` global variables. CHECK asserts the per-field
// flattened names and the sequential `location = N` assignment the
// legalizer emits, and that SV_Position is routed to the `gl_Position`
// builtin rather than a user `out` slot.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage vertex

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

// Inputs flatten to three `in` globals (one per VIn field).
//GLSL-DAG: in vec3 input_pos
//GLSL-DAG: in vec2 input_uv
//GLSL-DAG: in vec4 input_color
// User-semantic outputs flatten to `out` globals; SV_Position routes to gl_Position.
//GLSL-DAG: out vec2 entryPointParam_main_uv
//GLSL-DAG: out vec4 entryPointParam_main_color
//GLSL-DAG: gl_Position = o_{{[0-9]+}}.pos_{{[0-9]+}};
