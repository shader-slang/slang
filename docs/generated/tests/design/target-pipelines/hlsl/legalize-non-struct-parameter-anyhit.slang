//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizenonstructparametertostructforhlsl
//META: doc_section_digest=22ad05f0b82be681c3aaa0f7d4e69c88ae0c09144722950713aebddf79498241
//META: purpose=legalizeNonStructParameterToStructForHLSL wraps non-struct DXR-stage parameters in single-field struct types so DXC accepts them; the HLSL emission of an anyhit entry point taking a non-struct payload still produces a struct-typed payload parameter.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `### legalizeNonStructParameterToStructForHLSL` section:
// DXC requires DXR shader-stage parameters (anyhit, closesthit, etc.)
// to be struct types. A non-struct `inout float4` payload is wrapped
// in a single-field struct and unwrapped inside the entry point. The
// HLSL emit therefore contains a struct declaration that wraps the
// float4 plus an `anyhit` `void main(...)` entry-point function.

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
