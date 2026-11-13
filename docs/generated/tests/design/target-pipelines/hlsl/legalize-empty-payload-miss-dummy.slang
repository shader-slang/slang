//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:38:44Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeemptyraypayloadsforhlsl
//META: doc_section_digest=51ddf0e5476f0247fc85dfbdded23069c96869eb2f57ab744baf38d1fc99d983
//META: purpose=A miss shader whose payload parameters are all empty is emitted with an inout parameter of the separate DummyRayPayload struct, not the empty source type.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the receiving-shader arm of the empty-payload handling for a
// `miss` entry point: the empty source struct is replaced at the native
// interface by the separate one-uint `DummyRayPayload`. CHECK captures the
// dummy struct and requires the miss signature to use it; CHECK-NOT keeps
// the empty source type out of the signature.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage miss -entry main

struct Empty
{
}

[shader("miss")]
void main(inout Empty p)
{
}

// CHECK: struct [[PAYLOAD:DummyRayPayload[A-Za-z0-9_]*]]
// CHECK-NEXT: {
// CHECK-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// CHECK-NEXT: };
// CHECK: [shader("miss")]
// CHECK-NOT: Empty
// CHECK: void main(inout [[PAYLOAD]] {{[A-Za-z0-9_]+}})
