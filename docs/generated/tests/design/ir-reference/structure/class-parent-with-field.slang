//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#struct-internals
//META: doc_section_digest=b9a3eeb4f30459f951dba340f8e24ccd1d1bf7aa9598b948a2a6457d2453d958
//META: purpose=A class declaration lowers to a class parent opcode, a distinct spelling from struct, owning the same field children.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the container role is shared but the opcode spelling is not:
// the class-vs-struct distinction survives lowering. CHECK requires the header
// to read `class %Counter : Type` (not `struct`) while the child row is the
// same `field(key, type)` form a struct uses.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int seed;

class Counter
{
    int n;
}

[numthreads(1, 1, 1)]
void main()
{
    Counter c = new Counter();
    c.n = seed;
    buf[0] = c.n;
}

// CHECK: class %Counter	: Type
// CHECK: field(%n, Int)
