//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=1e957ad40dedfd3c52a2ed8eb2c7dd1be52c049778109f377a9197486f43c5df
//META: purpose=Expansion: `select` with widest-rank integer arms lowers to `select(%c, %a, %b)` with `Int64` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `select` opcode is unchanged at the 64-bit
// integer rank.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform int64_t a;
uniform int64_t b;
uniform RWStructuredBuffer<int64_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(c, a, b);
}

// IR-LABEL: func %main
// IR: select(%c, %a, %b)
