//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Stress: five-deep struct nesting produces five distinct `struct %Name` parent opcodes.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// five-deep struct nesting produces five distinct `struct %Name`
// parent opcodes. Five levels of nesting surface five distinct
// `struct` opcodes in the IR dump.

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
