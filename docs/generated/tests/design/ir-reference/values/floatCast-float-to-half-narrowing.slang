//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e9d21ff4a25410c2c41b9f2413bf081c7a9f699c2230818fe1a6fdf8979075a6
//META: purpose=Boundary: precision-narrowing `float -> half` lowers to a single `floatCast(%fa)` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `floatCast` covers precision changes in both
// directions. This boundary probe isolates the narrowing edge:
// 32-bit `float` to 16-bit `half`. The widening case (float to
// double) lives in the smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<half> hbuf;

[numthreads(1, 1, 1)]
void main()
{
    hbuf[0] = half(fa);
}

// IR-LABEL: func %main
// IR: floatCast(%fa)
