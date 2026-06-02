//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Expansion: unsigned `uint -> float` lowers to `castIntToFloat(%a)` with `Float` result — same opcode as the signed case, signedness encoded by operand type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Unsigned-to-float complement to the signed case. The doc
// admits one opcode `castIntToFloat`; the operand type carries
// the signedness.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(a);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%a)
