//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Boundary: `uint8_t(256)` (one past unsigned-8 MAX) constant-folds to a `0 : UInt8` payload, demonstrating unsigned modular wrap on the narrowing path.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc covers the literal-payload encoding for each integer
// width. `uint8_t(255)` is the MAX edge and survives as
// `255 : UInt8`; `uint8_t(256)` is one past the MAX and lowers as
// `0 : UInt8` — the wrap is observed inline on the store.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint8_t(255);
    buf[1] = uint8_t(256);
}

// IR-LABEL: func %main
// IR: store(%{{.*}}, 255 : UInt8)
// IR: store(%{{.*}}, 0 : UInt8)
