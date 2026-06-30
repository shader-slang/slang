//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
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
