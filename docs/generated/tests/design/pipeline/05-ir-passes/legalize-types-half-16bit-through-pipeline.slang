//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=Boundary: a 16-bit `half` value routed through type-legalization emits `OpCapability Float16` on SPIR-V and preserves the `half` spelling on HLSL.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `half` is a 16-bit float. The type-legalization passes carry the
// narrower bit-width through to the backend. On SPIR-V this surfaces
// as a `Float16` capability; on HLSL the `half` spelling survives.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform half a;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + half(1.0);
}

// SPIRV: OpCapability Float16
// SPIRV: OpEntryPoint
// HLSL: half
