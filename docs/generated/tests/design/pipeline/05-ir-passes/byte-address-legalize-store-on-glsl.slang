//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=Expansion: byte-address-legalize handles the store side of `RWByteAddressBuffer.Store<int>(addr, v)` — the store lowers to a typed indexed write into `_data[]` on GLSL.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Store-side counterpart of the byte-address-legalize claim. The
// `RWByteAddressBuffer.Store<int>(...)` call lowers to a typed
// `_data[...] = ...` write on the GLSL SSBO.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWByteAddressBuffer babuf;
uniform int v;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    babuf.Store<int>(0, v + int(tid.x));
}

// GLSL: layout(std430
// GLSL: babuf
// GLSL: _data[
