//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Boundary: documented `uint8_t` MAX (255) and lower-edge (0) survive lowering as `255 : UInt8` and `0 : UInt8` on the `store` operands.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Both edges of the unsigned-8 range on the literal-payload axis.
// The IR dump shows each integer literal as `<N> : UInt8` directly
// on the store opcode that writes the value into the buffer.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint8_t(0);
    buf[1] = uint8_t(255);
}

// IR-LABEL: func %main
// IR: store(%{{.*}}, 0 : UInt8)
// IR: store(%{{.*}}, 255 : UInt8)
