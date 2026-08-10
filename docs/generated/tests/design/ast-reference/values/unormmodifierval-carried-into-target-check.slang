//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#modifier-values
//META: doc_section_digest=4536635b38a6466b02bda12ed26c1d3f85e08898f1c387eaf21ba05a06ce719f
//META: purpose=A resource-format modifier becomes part of the texture's element type rather than being dropped after checking, so a target whose texture element types are restricted rejects the modified spelling that it accepts unmodified.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifier value really is stored on the type by
// showing it changes a later type-level decision. WGSL accepts
// `Texture2D<float4>`, so the only difference here is the `unorm`
// written on the element type; CHECK pins the resulting texture-element
// check failure (E41400). The diagnostic originates inside the core
// module's texture check, so no caret annotation is possible in this
// file and the match is on the code plus its message text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -stage compute -entry main

Texture2D<unorm float4> tex;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tex.Load(int3(0, 0, 0));
}

// CHECK: E41400
