//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=8f96c600fb4e9a50947a60ac22318e6fba6c6386abe0fabef983a2b0e0a9b785
//META: purpose=Doubly-nested instantiation Box<Box<int>> lowers to a nested specialize(%Box, specialize(%Box, Int)) type-level value.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on recursive generic instantiation: depth 2. The
// doc's `### Generic application` row's "Applies generic
// arguments to a generic value" claim allows the argument
// itself to be a `specialize` value — nesting therefore
// produces nested `specialize(%Box, specialize(%Box, Int))`
// type-level applications.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Box<T> { T val; };

uniform RWStructuredBuffer<int> buf;
uniform int a;

[numthreads(1, 1, 1)]
void main()
{
    Box<Box<int>> nested;
    nested.val.val = a;
    buf[0] = nested.val.val;
}

// CHECK: specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, Int))
