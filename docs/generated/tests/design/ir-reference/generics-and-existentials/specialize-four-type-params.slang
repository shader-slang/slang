//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=1523536872ebc337e174666c5cf954c332dd4d3653ef98ebc19e0608a020c147
//META: purpose=A call to a 4-type-param generic produces specialize(%base, T1, T2, T3, T4).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary at a wider arity than the two-parameter case, with a repeated type
// (`Int` in both first and last position) so the CHECK also confirms that the
// argument tail is positional rather than de-duplicated by value.
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
