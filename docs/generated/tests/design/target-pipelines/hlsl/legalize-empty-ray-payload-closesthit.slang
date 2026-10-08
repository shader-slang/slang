//META: generated=true
//META: model=gpt-6
//META: generated_at=2026-09-29T07:48:53+00:00
//META: source_commit=3bb522221d5db85fbdf8dcf441c05ffc3b801358
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeemptyraypayloadsforhlsl
//META: doc_section_digest=6e1f6ab0371a9b70d69e547a9aad8768979f0a53035570a77165208b8814dd76
//META: purpose=An empty logical ray payload is represented by a separate one-uint physical payload in the emitted HLSL closesthit entry-point signature.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An empty source payload still needs physical storage at the HLSL
// closest-hit interface. Capture the separate dummy struct, require its
// single uint field, and check that main uses that type for its inout
// payload. This ties the nonempty storage to the receiving entry point.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage closesthit -entry main

// Keep the logical source type empty; only its native interface needs
// the separate physical representation.
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

// CHECK: struct [[PAYLOAD:DummyRayPayload[A-Za-z0-9_]*]]
// CHECK-NEXT: {
// CHECK-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// CHECK-NEXT: };
// CHECK: [shader("closesthit")]
// CHECK: void main(inout [[PAYLOAD]] {{[A-Za-z0-9_]+}},
