//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=dd2ec8aef79db3f253a059c2cf01af45d95108a02c8daa3922585ed0db6730ef
//META: purpose=Expansion: a struct field of Int16 lowers to a field(...) row whose type operand is Int16.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's struct row says each field carries its IR type. Two
// `int16_t` fields surface two `field(%key, Int16)` rows.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Coord { int16_t x; int16_t y; }

uniform Coord c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(c.x) + int(c.y);
}

// IR-LABEL: struct %Coord
// IR: field(%{{.*}}, Int16)
