//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=Expansion: legalize-empty-array elides a zero-length fixed-size array struct field — the field name does not survive in the HLSL emit because the legalizer dropped the empty array.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Legalize empty array" row of the type-and-value legalization
// table (`slang-ir-legalize-empty-array.cpp`) "avoids zero-length
// arrays". A struct field declared as `int padding[0]` is dropped
// before emit — the struct can still be packed into a uniform buffer
// but the `padding` name does not survive.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct WithEmpty
{
    int x;
    int padding[0];
    int y;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    WithEmpty w;
    w.x = a;
    w.y = a + 1;
    buf[0] = w.x + w.y;
}

// HLSL: void main
// HLSL-NOT: padding
