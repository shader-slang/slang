//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Expansion: `bit_cast<float>(int)` signed-int-to-float reinterpret lowers to `bitCast(%a)` with `Float` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Counterpart to the existing `bit_cast<float>(uint)` test: same
// width but signed source. Verifies that lowering keeps signed and unsigned
// int-to-float bit-reinterprets under one `bitCast` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<float>(a);
}

// IR-LABEL: func %main
// IR: bitCast(%a)
