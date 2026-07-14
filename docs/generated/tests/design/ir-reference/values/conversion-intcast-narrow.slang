//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=`intCast` is the integer-to-integer conversion opcode; both narrowing (int -> int16_t) and widening (int -> uint64_t) lower through it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `intCast` for "Integer-to-integer cast (sign/zero
// extension chosen by types)". The cross-cutting bundle covers
// `int(uint)`; this test exercises narrowing and widening + 64-bit
// extension to confirm the same opcode handles all of them.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int16_t> sbuf;
uniform RWStructuredBuffer<uint64_t> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    sbuf[0] = int16_t(a);
    ubuf[0] = uint64_t(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
// IR: intCast(%a)
