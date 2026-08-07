//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
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
