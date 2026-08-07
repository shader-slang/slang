//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=A generic with a compile-time integer value parameter lowers to specialize(%base, N : Int) — the literal appears in the argument list.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on argument *kind*: a generic argument need not be a type. A
// value-parameter generic instantiated at `4` puts the integer literal
// directly into the argument list, and the CHECK pins `4 : Int` in that
// position.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct FixedArr<int N> { int data[N]; };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    FixedArr<4> arr;
    arr.data[0] = inVal;
    buf[0] = arr.data[0];
}

// CHECK: specialize(%{{[0-9]+}}, 4 : Int)
