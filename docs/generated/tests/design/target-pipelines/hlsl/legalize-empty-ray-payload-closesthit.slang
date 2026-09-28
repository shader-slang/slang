//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeemptyraypayloadsforhlsl
//META: doc_section_digest=6e1f6ab0371a9b70d69e547a9aad8768979f0a53035570a77165208b8814dd76
//META: purpose=legalizeEmptyRayPayloadsForHLSL adds a dummy field to an empty DXR ray-payload struct so DXIL has a non-empty payload; the HLSL emission of a closesthit entry point keeps a non-empty payload struct.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an empty `[raypayload]` struct is given a synthetic dummy
// field before HLSL emit, because DXIL + NVAPI cannot express a zero-sized
// payload. The test declares an empty payload and a `closesthit` entry point
// that takes it `inout`; CHECK asserts the emit still carries a payload
// struct declaration and the closesthit function, which is only possible if
// the legalization added a member rather than dropping the parameter.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage closesthit -entry main

// The legalizeEmptyRayPayloadsForHLSL pass keys on
// `IRRayPayloadDecoration`, attached via the `[raypayload]` attribute
// in Slang surface syntax. Without the attribute the struct is not
// recognised as a payload and the legalization pass does not fire,
// so the payload + closesthit signature are dropped during lowering.
[raypayload]
struct EmptyPayload
{
}

[shader("closesthit")]
void main(
    inout EmptyPayload payload,
    BuiltInTriangleIntersectionAttributes attrs)
{
}

// CHECK-DAG: struct EmptyPayload
// CHECK-DAG: void main(
