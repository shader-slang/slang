//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=53ad1d566b89727f915f7a684edad14e039800bbbb1330d913ea0c67195afd17
//META: purpose=A parameter declared with an HLSL system-value semantic produces a semantic decoration with (name, index) operands; the index is -1 when none is written.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two-operand shape of `semantic`: a string name and an
// integer index. The entry-point parameter carries `SV_DispatchThreadID`
// with no index, and the CHECK pins `-1` as the second operand, which is
// how "no explicit index" is encoded.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [semantic("SV_DispatchThreadID", -1 : Int)]
