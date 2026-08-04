//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
//META: purpose=Expansion: lower-bit-cast routes `bit_cast<uint>(float)` to a SPIR-V `OpBitcast` instruction — alternative target axis to the HLSL/GLSL spellings.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V target axis on lower-bit-cast. The float-to-uint
// reinterpret surfaces as `OpBitcast` in the SPIR-V assembly emit.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform float f;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bit_cast<uint>(f);
}

// SPIRV: OpBitcast
