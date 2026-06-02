//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A generic with a compile-time integer value parameter lowers to specialize(%base, N : Int) — int literal appears in the operand list.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on generic value-parameter axis: `struct FixedArr<int N>`
// referenced as `FixedArr<4>` should produce `specialize(%FixedArr,
// 4 : Int)` — the per-doc `### specialize` row says operands are
// `base, arg, ...` without restricting to types, and the
// observed dump form for an integer literal arg is `N : Int`.

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
