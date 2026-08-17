//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=53ad1d566b89727f915f7a684edad14e039800bbbb1330d913ea0c67195afd17
//META: purpose=A function parameter declared with the precise modifier carries a precise decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `precise` row: a nullary decoration produced by the
// `precise` modifier. The modifier is written on a function parameter and
// the CHECK pins the bare `[precise]` line, showing the request is recorded
// on the declaration rather than folded into the arithmetic.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

float bumpy(precise float x) { return x * x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bumpy(float(tid.x));
}

// CHECK: [precise]
