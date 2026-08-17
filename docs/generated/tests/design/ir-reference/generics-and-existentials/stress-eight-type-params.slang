//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=1523536872ebc337e174666c5cf954c332dd4d3653ef98ebc19e0608a020c147
//META: purpose=An 8-type-parameter generic call still lowers to specialize(%base, T1..T8) with eight argument operands.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Stresses the variadic argument tail well past the two operands the schema
// names. Eight type parameters must all survive into one argument list; the
// CHECK spells all eight, so a truncated tail fails.
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
