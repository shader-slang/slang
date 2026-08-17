//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5f27d4bb5fbd1429627db68bec594389689c236998e7182eaaf350ae3ceda2ee
//META: purpose=Boundary: documented `int8_t` MIN (-128) survives lowering as `-128 : Int8` on the `store` operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower edge of the signed-8 range. The literal payload appears
// as `-128 : Int8` directly on the `store` of the narrow integer.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int8_t(127);
    buf[1] = int8_t(-128);
}

// IR-LABEL: func %main
// IR: store(%{{.*}}, 127 : Int8)
// IR: store(%{{.*}}, -128 : Int8)
