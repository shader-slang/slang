//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=dd2ec8aef79db3f253a059c2cf01af45d95108a02c8daa3922585ed0db6730ef
//META: purpose=Stress: five-deep struct nesting produces five distinct `struct %Name` parent opcodes.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the `struct` opcode is a parent container; nesting
// is by carrying one struct type as a field of another. Five
// levels of nesting surface five distinct `struct` opcodes in
// the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct A { int x; }
struct B { A inner; int y; }
struct C { B inner; int z; }
struct D { C inner; int w; }
struct E { D inner; int v; }

uniform E e;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = e.inner.inner.inner.inner.x;
}

// IR: struct %A
// IR: struct %B
// IR: struct %C
// IR: struct %D
// IR: struct %E
