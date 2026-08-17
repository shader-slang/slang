//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=`intCast` is the integer-to-integer conversion opcode; both narrowing (int -> int16_t) and widening (int -> uint64_t) lower through it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `intCast` is the single integer-to-integer cast, with
// sign or zero extension chosen by the types. The cross-cutting bundle covers
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
