//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:38:44Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizenonstructparametertostructforhlsl
//META: doc_section_digest=709ba20230a3c7f123762b400712ee8e3976f798151abee31576df09be5bcf22
//META: purpose=A non-empty non-struct payload of a D3D anyhit entry point is retyped to a generated one-field wrapper struct whose field is named data, so the emitted parameter is struct-typed.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an anyhit entry point whose inout payload is a bare
// `float4` is emitted with a generated one-field wrapper struct: the
// wrapper is `RayPayload_t` with a `data` field, the parameter takes the
// wrapper, and the body reaches the value through `.data`. CHECK captures
// the wrapper name once and ties the struct declaration to the signature.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage anyhit -entry main

[shader("anyhit")]
void main(
    inout float4 payload,
    BuiltInTriangleIntersectionAttributes attrs)
{
    payload.x = payload.x + 1.0;
}

// CHECK: struct [[WRAP:RayPayload_t[A-Za-z0-9_]*]]
// CHECK-NEXT: {
// CHECK-NEXT: float4 data{{[A-Za-z0-9_]*}};
// CHECK: [shader("anyhit")]
// CHECK: void main(inout [[WRAP]] {{[A-Za-z0-9_]+}},
// CHECK: .data
