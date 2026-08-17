//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=1523536872ebc337e174666c5cf954c332dd4d3653ef98ebc19e0608a020c147
//META: purpose=A generic struct's conformance lowers to a generic witness table, so the witness argument at a call site is itself a specialize of that generic.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the third base kind a `specialize` can apply to: a generic witness
// table. `Box<T> : IFoo` makes the conformance itself generic, so the dump
// carries `generic %N : witness_table_t(%IFoo)`. Calling a constrained helper
// with `Box<int>` must then pass a specialized witness, and the CHECK asserts
// the call's argument list holds `specialize(<witness generic>, Int)` next to
// the specialized type argument.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };
struct Box<T> : IFoo { T v; int run(int x) { return x + 1; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int use<U : IFoo>(U u, int x) { return u.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    Box<int> b;
    b.v = inVal;
    buf[0] = use<Box<int> >(b, inVal);
}

// CHECK: generic %{{[0-9]+}} : witness_table_t(%IFoo)
// CHECK: call specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, Int), specialize(%{{[0-9]+}}, Int))
