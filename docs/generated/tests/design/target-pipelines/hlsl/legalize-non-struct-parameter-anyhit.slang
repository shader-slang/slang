//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizenonstructparametertostructforhlsl
//META: doc_section_digest=73eddbf92f9a1dd3768bc3f8d75a4776ee86e6cd3d0c38eeda0ae88faa4bf883
//META: purpose=legalizeNonStructParameterToStructForHLSL wraps non-struct DXR-stage parameters in single-field struct types so DXC accepts them; the HLSL emission of an anyhit entry point taking a non-struct payload still produces a struct-typed payload parameter.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a DXR entry point whose payload parameter is not a struct
// gets that parameter wrapped in a generated single-field struct, because DXC
// only accepts struct-typed DXR-stage parameters. The test declares an
// `anyhit` entry point over a payload carrying a bare `float4`; CHECK asserts
// the emit contains a struct declaration holding the float4 and an
// entry-point signature that takes it, i.e. the wrapped-and-unwrapped shape
// the legalization produces.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile lib_6_6 -stage anyhit -entry main

struct ScalarPayload
{
    float4 color;
}

[shader("anyhit")]
void main(
    inout ScalarPayload payload,
    BuiltInTriangleIntersectionAttributes attrs)
{
    payload.color.x = payload.color.x + 1.0;
}

// CHECK: struct ScalarPayload
// CHECK: void main(
