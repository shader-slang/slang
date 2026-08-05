//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#modifier-values
//META: doc_section_digest=4536635b38a6466b02bda12ed26c1d3f85e08898f1c387eaf21ba05a06ce719f
//META: purpose=A resource-format modifier written on a texture element type is converted into a value stored on the modified type, and that stored modifier reaches the emitted HLSL element spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `unorm` written inside the texture's type argument
// is not discarded once the type expression is checked: it becomes part
// of the texture's element type and travels with it all the way to
// codegen. CHECK asserts the emitted HLSL declaration still spells
// `unorm` in the element position, which it can only do if the modifier
// was carried on the type rather than on the declaration syntax.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

Texture2D<unorm float4> tex;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tex.Load(int3(0, 0, 0));
}

// CHECK: Texture2D<unorm float4
