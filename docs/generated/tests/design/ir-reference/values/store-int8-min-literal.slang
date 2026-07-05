//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
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
