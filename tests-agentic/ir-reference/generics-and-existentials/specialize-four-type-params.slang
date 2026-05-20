//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A call to a 4-type-param generic produces specialize(%base, T1, T2, T3, T4) — operand list scales with arity per `### specialize`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on generic arity: 4 type parameters. Per the doc's
// `### specialize` operand layout `base, arg, ...`, the call
// lowers to `call specialize(%base, T1, T2, T3, T4)(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform float b;
uniform uint c;
uniform int d;
uniform RWStructuredBuffer<int> buf;

A first4<A, B, C, D>(A x, B y, C z, D w) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = first4<int, float, uint, int>(a, b, c, d);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int, Float, UInt, Int)
