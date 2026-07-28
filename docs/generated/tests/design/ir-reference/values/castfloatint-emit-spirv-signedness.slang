//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=`castFloatToInt` lowers to `OpConvertFToS`/`OpConvertFToU` keyed by the target integer's signedness, and `castIntToFloat` lowers to `OpConvertSToF`/`OpConvertUToF` keyed by the source integer's signedness.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `castFloatToInt` ("Float-to-int conversion (truncation)")
// and `castIntToFloat` ("Int-to-float conversion") as single opcodes. Each
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
