//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=Boundary: a `uint8_t` (narrow 8-bit) value routed through the type-legalization pipeline emits `OpCapability Int8` on SPIR-V.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An 8-bit unsigned integer is one of the narrow scalar types whose
// representation must be carried through type / value legalization to
// the backend. On SPIR-V the legalizer requests `Int8` and
// `UniformAndStorageBuffer8BitAccess` capabilities.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform uint8_t a;
uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + uint8_t(1);
}

// SPIRV: OpCapability Int8
// SPIRV: OpEntryPoint
