//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=e7e100a63c866d0df80fc71ebcb3d885933673afb6bac3a23c94ac816f7b50b1
//META: purpose=A reference to a generic struct at a concrete type produces a specialize(type, T) type-level value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that `specialize` applies to a generic *type*, not only to a
// generic function. A local of type `Pair<int>` forces the generic struct to
// be instantiated; the CHECK asserts that the type this local carries is
// spelled `specialize(<Pair>, Int)` in the dump, which is the type-application
// form of the same opcode.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

struct Pair<T> { T a; T b; };

T sumOf<T : __BuiltinIntegerType>(Pair<T> p) { return p.a + p.b; }

[numthreads(1, 1, 1)]
void main()
{
    Pair<int> p;
    p.a = inVal;
    p.b = inVal + 1;
    buf[0] = sumOf<int>(p);
}

// CHECK: specialize(%{{[0-9]+}}, Int)
