//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Expansion: a struct field of UInt64 lowers to a field(...) row carrying UInt64.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a struct field of UInt64 lowers to a field(...) row carrying UInt64.
// A `uint64_t` field surfaces a `field(%key, UInt64)` row.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Handle64 { uint64_t opaque; }

uniform Handle64 h;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(h.opaque);
}

// IR-LABEL: struct %Handle64
// IR: field(%{{.*}}, UInt64)
