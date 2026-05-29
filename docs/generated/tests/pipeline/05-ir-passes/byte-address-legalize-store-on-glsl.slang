//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
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
