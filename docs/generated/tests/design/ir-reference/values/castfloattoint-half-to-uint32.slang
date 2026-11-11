//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e9d21ff4a25410c2c41b9f2413bf081c7a9f699c2230818fe1a6fdf8979075a6
//META: purpose=Expansion: `half -> uint` truncation lowers to a single `castFloatToInt(%ha)` with `UInt` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Smallest-rank float to mid-rank unsigned int. Verifies that lowering keeps
// it under one opcode keyed by operand and result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform half ha;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(ha);
}

// IR-LABEL: func %main
// IR: castFloatToInt(%ha)
