//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Expansion: signed 64-to-32 truncation lowers to a single `intCast(%i64a)` with `Int` result — the reverse direction of the sign-extend widening case.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Top-of-rank truncation: 64-bit signed to 32-bit signed. The
// doc places this under the same `intCast` opcode as widening.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t i64a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(i64a);
}

// IR-LABEL: func %main
// IR: intCast(%i64a)
