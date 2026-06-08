//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=An 8-type-parameter generic call still lowers to specialize(%base, T1..T8) with eight argument operands.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress on generic arity: 8 type parameters. The doc's
// `### specialize` row's operand layout `base, arg, ...` is
// unbounded — an 8-parameter generic should produce an 8-arg
// `specialize` operand list with no compiler-imposed cap.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

A pickFirst<A, B, C, D, E, F, G, H>(A x1, B x2, C x3, D x4, E x5, F x6, G x7, H x8)
{
    return x1;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickFirst<int, int, int, int, int, int, int, int>(a, a, a, a, a, a, a, a);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int, Int, Int, Int, Int, Int, Int, Int)
