//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=b3e088ac3dc9706ad8749cb8f9a4f8033e0bd510147eb7d3d1d0a3563b2dd6ca
//META: purpose=Expansion: widening conversion from UInt8 to UInt yields a UInt-typed intCast value.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// widening conversion from UInt8 to UInt yields a UInt-typed intCast
// value. A `uint(v)` widening from a `uint8_t` value produces an
// `intCast` whose result type is `UInt`, complementing the narrow
// direction exercised by other expansion tests.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint8_t v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    uint widened = uint(v);
    buf[0] = widened;
}

// IR: let {{.*}}%widened{{.*}}: UInt{{.*}}= intCast(%v)
