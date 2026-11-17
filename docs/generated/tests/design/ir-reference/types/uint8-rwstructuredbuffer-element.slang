//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=8a7a3c63487b8f293c70c075134ef33eae10ff00b2e4f1c4f9b195bed7528e0c
//META: purpose=Expansion: RWStructuredBuffer parameterised by UInt8 names the narrow scalar as its first operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWStructuredBuffer parameterised by UInt8 names the narrow scalar as
// its first operand. A `RWStructuredBuffer<uint8_t>` shows
// `RWStructuredBuffer(UInt8, ...)` for the global_param.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint8_t(0xFF);
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(UInt8{{.*}}= global_param
