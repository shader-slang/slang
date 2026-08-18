//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Boundary: widest float-widening `half -> double` lowers to a `floatCast(%ha)` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lowering lumps any precision change under `floatCast`. This
// boundary probe takes the widest available widening step in the
// documented type ladder: `half` (16-bit) to `double` (64-bit).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half ha;
uniform RWStructuredBuffer<double> dbuf;

[numthreads(1, 1, 1)]
void main()
{
    dbuf[0] = double(ha);
}

// IR-LABEL: func %main
// IR: floatCast(%ha)
