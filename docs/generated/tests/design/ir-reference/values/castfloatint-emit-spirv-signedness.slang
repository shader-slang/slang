//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=`castFloatToInt` lowers to `OpConvertFToS`/`OpConvertFToU` keyed by the target integer's signedness, and `castIntToFloat` lowers to `OpConvertSToF`/`OpConvertUToF` keyed by the source integer's signedness.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the truncating `castFloatToInt` and the widening
// `castIntToFloat` are each a single IR opcode. Each
// splits in SPIR-V on the integer side's signedness: float->signed emits
// `OpConvertFToS`, float->unsigned `OpConvertFToU`, signed->float
// `OpConvertSToF`, unsigned->float `OpConvertUToF`. All four cells are
// distinct; CHECK pins each. Operands are uniforms so the casts survive.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform float f;
uniform int si;
uniform uint ui;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = int(f);
    ubuf[0] = uint(f);
    fbuf[0] = float(si);
    fbuf[1] = float(ui);
}

// SPV-DAG: OpConvertFToS %int %{{[0-9]+}}
// SPV-DAG: OpConvertFToU %uint %{{[0-9]+}}
// SPV-DAG: OpConvertSToF %float %{{[0-9]+}}
// SPV-DAG: OpConvertUToF %float %{{[0-9]+}}
