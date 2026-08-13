//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Expansion: struct field of UInt8 surfaces a field(...) row carrying UInt8.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// struct field of UInt8 surfaces a field(...) row carrying UInt8. A
// struct holding a `uint8_t` field produces a `field(%key, UInt8)` row
// in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Byte { uint8_t value; }

uniform Byte b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(b.value);
}

// IR-LABEL: struct %Byte
// IR: field(%{{.*}}, UInt8)
