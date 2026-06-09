//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=dd2ec8aef79db3f253a059c2cf01af45d95108a02c8daa3922585ed0db6730ef
//META: purpose=A user struct lowers to a parent `struct %Name` opcode containing `field(%key, <T>)` rows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Struct and class containers" table notes that the
// `struct` opcode "acts both as a type and as a parent container
// for their field and key children." A user struct with mixed
// scalar/vector fields surfaces the parent opcode plus one `field`
// row per declared member.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Mixed { int x; float y; bool z; }

uniform Mixed s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (s.z ? 1.0 : 0.0) + float(s.x) + s.y;
}

// IR-LABEL: struct %Mixed
// IR: field(%{{.*}}, Int)
// IR: field(%{{.*}}, Float)
// IR: field(%{{.*}}, Bool)
