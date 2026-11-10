//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#hlsl-semantics
//META: doc_section_digest=924ecf08334941e0ce19332c62256f9af73bd09c738c20e634f3eefd0c64d11f
//META: purpose=`read(...)` / `write(...)` ray-payload access semantics on a payload field survive to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ray-payload access semantics, which unlike `register` /
// `packoffset` carry a list of stage names rather than a binding. A closesthit
// entry point consumes a payload whose field declares both accessors, and the
// CHECK-DAG lines assert the `[raypayload]` struct decoration and both stage
// lists reappear verbatim in the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile lib_6_7 -stage closesthit -entry chitMain

[raypayload]
struct ColorPayload
{
    float4 color : read(caller, anyhit) : write(closesthit);
};

[shader("closesthit")]
void chitMain(inout ColorPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float4(1.0f, 0.5f, 0.25f, 1.0f);
}

//CHECK-DAG: [raypayload]
//CHECK-DAG: read(caller, anyhit)
//CHECK-DAG: write(closesthit)
