//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=dd2ec8aef79db3f253a059c2cf01af45d95108a02c8daa3922585ed0db6730ef
//META: purpose=Expansion: a struct field of UInt64 lowers to a field(...) row carrying UInt64.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's struct row says fields carry their IR type. A
// `uint64_t` field surfaces a `field(%key, UInt64)` row.

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
