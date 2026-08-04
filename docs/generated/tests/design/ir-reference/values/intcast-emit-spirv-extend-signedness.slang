//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=The `intCast` opcode lowers to `OpSConvert` when widening a signed integer and `OpUConvert` when widening an unsigned integer, realizing the doc's "sign/zero extension chosen by types".
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc describes `intCast` as "Integer-to-integer cast (sign/zero
// extension chosen by types)". The unified IR opcode splits in SPIR-V on
// the source signedness when widening: a signed source emits `OpSConvert`
// (sign-extend), an unsigned source emits `OpUConvert` (zero-extend).
// This pins the distinguishing op per signedness row. Operands are
// uniform narrow ints to keep the conversion live.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform int8_t s8;
uniform uint8_t u8;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = int(s8);
    ubuf[0] = uint(u8);
}

// SPV-DAG: OpSConvert %int %{{[0-9]+}}
// SPV-DAG: OpUConvert %uint %{{[0-9]+}}
