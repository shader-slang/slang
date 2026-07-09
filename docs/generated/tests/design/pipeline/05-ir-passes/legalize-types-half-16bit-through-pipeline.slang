//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
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
