//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5f27d4bb5fbd1429627db68bec594389689c236998e7182eaaf350ae3ceda2ee
//META: purpose=Boundary: documented `int64_t` MAX (9223372036854775807) survives lowering as `9223372036854775807 : Int64` on the `add` operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Upper edge of the signed-64 range. The integer literal stays in
// 64-bit form and rides as the inline payload of an `add`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t a;
uniform RWStructuredBuffer<int64_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 9223372036854775807;
}

// IR-LABEL: func %main
// IR: add(%a, 9223372036854775807 : Int64)
