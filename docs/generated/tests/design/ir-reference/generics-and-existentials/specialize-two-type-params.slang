//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=A call to a 2-type-param generic produces specialize(%base, T1, T2) — the argument list grows with arity.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on generic arity: with two type parameters the argument tail is two
// entries long and keeps source order. The CHECK pins `Int, Float` in the order
// written at the call site, so a reordering or a collapsed tail would fail.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

A pickFirst<A, B>(A x, B y) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickFirst<int, float>(a, 1.0f);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int, Float)
