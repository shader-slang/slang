//META: generated=true
//META: model=claude-opus-5-5[1m]
//META: generated_at=2026-09-29T08:41:41+00:00
//META: source_commit=b9199bdaa48162d97fa8f9d3293e6edaf037ffb3
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeemptyraypayloadsforhlsl
//META: doc_section_digest=6e1f6ab0371a9b70d69e547a9aad8768979f0a53035570a77165208b8814dd76
//META: purpose=An empty DXR ray-payload parameter of a closesthit entry point still reaches HLSL as a non-empty inout payload struct, which DXC requires of a closesthit entry point.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an empty closesthit payload still reaches HLSL as the non-empty
// `inout` struct DXC requires. The source struct is not padded; a new one-member
// type replaces it, so CHECK captures the first emitted struct's name, requires
// one member, and requires `main` to take that struct `inout`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage closesthit -entry main

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

// CHECK: struct [[PAYLOAD:[A-Za-z_0-9]+]]
// CHECK-NEXT: {
// CHECK-NEXT: {{[A-Za-z_0-9]+}} {{[A-Za-z_0-9]+}};
// CHECK-NEXT: };
// CHECK: void main(inout [[PAYLOAD]] {{[A-Za-z_0-9]+}}, BuiltInTriangleIntersectionAttributes
