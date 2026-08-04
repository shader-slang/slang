//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=b49b3d499615e99101dda545c0f7fef50e62178dcf5f11f8dcef9ff0becec030
//META: purpose=Expansion: a struct field of UInt16 lowers to a field(...) row carrying UInt16.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's struct row says fields carry their IR type. A
// `uint16_t` field surfaces a `field(%key, UInt16)` row.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Span { uint16_t start; uint16_t len; }

uniform Span s;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(s.start) + uint(s.len);
}

// IR-LABEL: struct %Span
// IR: field(%{{.*}}, UInt16)
