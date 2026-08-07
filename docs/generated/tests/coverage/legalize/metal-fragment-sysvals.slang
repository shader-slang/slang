//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the Metal system-value table for fragment inputs and outputs, mapping SV_Position/SV_IsFrontFace/SV_SampleIndex/SV_PrimitiveID/SV_RenderTargetArrayIndex/SV_ViewportArrayIndex/SV_ViewID onto Metal attribute qualifiers and SV_Target/SV_DepthGreaterEqual/SV_Coverage/SV_StencilRef onto the output struct.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `LegalizeMetalEntryPointContext::getSystemValueInfo`
// (slang-ir-legalize-varying-params.cpp:4110). Metal expresses varying
// parameters as `[[attribute]]` qualifiers rather than global builtins, so
// this table maps each `SV_*` name to a Metal attribute string and records
// the types Metal will accept for it. Two mappings in the table are not
// obvious renames and are worth pinning by name: `SV_ViewID` becomes
// `[[amplification_id]]` (Metal has no separate multi-view index), and
// `SV_DepthGreaterEqual` becomes the parameterised `[[depth(greater)]]`
// rather than a distinct attribute. CHECK asserts the whole flattened entry
// point signature, which shows each input parameter with its attribute in
// order, plus the four output-struct members.
//TEST:SIMPLE(filecheck=METAL):-target metal -entry fragMain -stage fragment

struct FIn
{
    float4 pos : SV_Position;
    bool front : SV_IsFrontFace;
    uint sampleId : SV_SampleIndex;
    uint prim : SV_PrimitiveID;
    uint layer : SV_RenderTargetArrayIndex;
    uint vp : SV_ViewportArrayIndex;
    uint view : SV_ViewID;
}
struct FOut
{
    float4 color : SV_Target0;
    float depth : SV_DepthGreaterEqual;
    uint cover : SV_Coverage;
    uint stencil : SV_StencilRef;
}

[shader("fragment")]
FOut fragMain(FIn i)
{
    FOut o;
    o.color = i.pos + float(i.front) + float(i.sampleId + i.prim + i.layer + i.vp + i.view);
    o.depth = 0.5;
    o.cover = 1;
    o.stencil = 3;
    return o;
}

// Fragment outputs become qualified members of the returned struct.
//METAL: struct FOut_0
//METAL-DAG: float4 color_0 {{\[\[}}color(0)]];
//METAL-DAG: float depth_0 {{\[\[}}depth(greater)]];
//METAL-DAG: uint cover_0 {{\[\[}}sample_mask]];
//METAL-DAG: uint stencil_0 {{\[\[}}stencil]];
// The input struct is flattened into one attributed parameter per field.
//METAL: {{\[\[}}fragment]] FOut_0 fragMain(float4 pos_0 {{\[\[}}position]], bool front_0 {{\[\[}}front_facing]], uint sampleId_0 {{\[\[}}sample_id]], uint prim_0 {{\[\[}}primitive_id]], uint layer_0 {{\[\[}}render_target_array_index]], uint vp_0 {{\[\[}}viewport_array_index]], uint view_0 {{\[\[}}amplification_id]])
