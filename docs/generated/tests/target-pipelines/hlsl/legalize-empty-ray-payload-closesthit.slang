//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeemptyraypayloadsforhlsl
//META: doc_section_digest=a1a0698d0601593c56d2536c58849e0fa5512ebb7c81aaaa30347f38484f6c9f
//META: purpose=legalizeEmptyRayPayloadsForHLSL adds a dummy field to an empty DXR ray-payload struct so DXIL has a non-empty payload; the HLSL emission of a closesthit entry point keeps a non-empty payload struct.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `### legalizeEmptyRayPayloadsForHLSL` section: DXR
// requires non-empty ray-payload structs. An empty `struct Payload {}`
// is rewritten with a synthetic dummy field. We define an empty
// payload and a `closesthit` entry point, and check that the HLSL
// emit retains a non-empty payload struct declaration (the canonical
// DXR shape) and the closesthit entry-point function.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage closesthit -entry main

struct EmptyPayload
{
}

[shader("closesthit")]
void main(
    inout EmptyPayload payload,
    BuiltInTriangleIntersectionAttributes attrs)
{
}

// CHECK: struct EmptyPayload
// CHECK: void main(
