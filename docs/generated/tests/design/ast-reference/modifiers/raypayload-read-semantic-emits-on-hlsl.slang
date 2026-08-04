//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#hlsl-semantics-sv
//META: doc_section_digest=54f4af64d5483b7505c99a89151017d4036d4772330ee479fadd0f341299c8f6
//META: purpose=`RayPayloadReadSemantic` / `RayPayloadWriteSemantic` access qualifiers on a `[raypayload]` struct field survive to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RayPayloadReadSemantic` (`: read(...)`) and `RayPayloadWriteSemantic`
// (`: write(...)`) are the AST nodes attached to ray-payload struct
// fields. The compiler propagates them to emitted HLSL for the closest-
// hit entry point that consumes the payload, including the
// `[raypayload]` struct decoration.

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

//CHECK: [raypayload]
//CHECK: read(caller, anyhit)
//CHECK: write(closesthit)
