//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=8f96c600fb4e9a50947a60ac22318e6fba6c6386abe0fabef983a2b0e0a9b785
//META: purpose=A 5-deep nested Box<Box<Box<Box<Box<int>>>>> lowers to five nested specialize(%Box, ...) applications.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress on recursive generic instantiation: depth 5. The
// doc's `### Generic application` claim that `specialize`
// applies to "a generic value" recurses — the leaf is `Int` and
// each level wraps the previous one in another `specialize(%Box,
// ...)`. Five wrapping levels should yield five nested
// `specialize` opcodes in the type expression.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Box<T> { T val; };

uniform RWStructuredBuffer<int> buf;
uniform int a;

[numthreads(1, 1, 1)]
void main()
{
    Box<Box<Box<Box<Box<int>>>>> deep;
    deep.val.val.val.val.val = a;
    buf[0] = deep.val.val.val.val.val;
}

// CHECK: specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, Int)))))
