//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Expansion: 16-bit same-width reinterpret `bit_cast<half>(uint16_t)` lowers to `bitCast(%u16a)` with `Half` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 16-bit half-width reinterpret. Verifies `bitCast` is the same
// opcode at the half-precision tier.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint16_t u16a;
uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<half>(u16a);
}

// IR-LABEL: func %main
// IR: bitCast(%u16a)
