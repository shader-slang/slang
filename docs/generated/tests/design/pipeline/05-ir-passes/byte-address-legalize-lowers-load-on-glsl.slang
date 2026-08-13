//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=The byte-address-legalize pass rewrites a typed `ByteAddressBuffer.Load<int>(0)` into a typed indexed access on GLSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Byte-address legalize" row of the type-and-value legalization
// table (`slang-ir-byte-address-legalize.cpp`) rewrites
// `ByteAddressBuffer.Load<T>(addr)` into a typed access. On GLSL the
// emitted form is a typed SSBO indexed read into `_data[...]`.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform ByteAddressBuffer babuf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = babuf.Load<int>(0);
}

// GLSL: layout(std430
// GLSL: babuf
// GLSL: _data[
