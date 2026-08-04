//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=Boundary: precision-narrowing `float -> half` lowers to a single `floatCast(%fa)` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `floatCast` covers precision changes in both
// directions. This boundary probe isolates the narrowing edge:
// 32-bit `float` to 16-bit `half`. The widening case (float to
// double) lives in the smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<half> hbuf;

[numthreads(1, 1, 1)]
void main()
{
    hbuf[0] = half(fa);
}

// IR-LABEL: func %main
// IR: floatCast(%fa)
