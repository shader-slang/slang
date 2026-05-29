//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=8f96c600fb4e9a50947a60ac22318e6fba6c6386abe0fabef983a2b0e0a9b785
//META: purpose=A reference to a generic struct at a concrete type produces a specialize(type, T) type-level value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Pair<int>` for `struct Pair<T> { T a; T b; }` produces a
// `specialize(%Pair, Int)` value in the IR — the doc's
// `### Generic application` claim that `specialize` applies to
// "a generic value (function, type, or witness)".

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
