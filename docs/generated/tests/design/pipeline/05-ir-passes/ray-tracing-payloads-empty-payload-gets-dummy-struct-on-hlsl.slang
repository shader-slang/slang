//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:50:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=b9ff903836e65c228e0e2f2b3aec5862dda6d78f824babeb56251f657789e6bf
//META: purpose=legalizeRayTracingPayloads gives an empty ray payload physical storage on D3D by substituting a nonempty dummy struct.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Legalize ray-tracing payloads row says an empty payload gets a
// nonempty dummy struct on D3D, GLSL and SPIR-V. The HLSL emit shows the
// user's empty `Payload` replaced by `DummyRayPayload_0` holding one uint.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_5 -stage closesthit -entry chit

struct Payload {};

[shader("closesthit")]
void chit(inout Payload p, in BuiltInTriangleIntersectionAttributes attr) {}

// HLSL: struct DummyRayPayload_{{[0-9]+}}
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy_{{[0-9]+}};
// HLSL: void chit(inout DummyRayPayload_{{[0-9]+}} p_{{[0-9]+}}
