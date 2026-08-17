//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: middle-of-ladder float widening `float -> double` lowers to `floatCast(%fa)` with `Double` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Middle rung of the float ladder, reversed: 32-bit to 64-bit
// widening. Verifies that lowering keeps both narrowing and widening under one
// `floatCast` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = double(fa);
}

// IR-LABEL: func %main
// IR: floatCast(%fa)
