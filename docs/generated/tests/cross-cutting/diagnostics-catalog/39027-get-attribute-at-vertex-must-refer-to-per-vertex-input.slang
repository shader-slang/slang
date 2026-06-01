//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39027 (get-attribute-at-vertex-must-refer-to-per-vertex-input) - 'GetAttributeAtVertex' must reference vertex input
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39027
//META: catalog_name=get-attribute-at-vertex-must-refer-to-per-vertex-input
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `GetAttributeAtVertex` requires a per-vertex / nointerpolation
// input directly as its first argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage fragment -entry main -profile sm_6_1

float4 main(float4 vColor : COLOR) : SV_Target
{
    return GetAttributeAtVertex(vColor, 0);
}

// CHECK: E39027
