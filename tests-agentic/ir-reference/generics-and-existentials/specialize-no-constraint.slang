//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A 0-constraint generic call lowers to specialize(%base, T) without any trailing witness operand — pairs with the constrained form's extra witness arg.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on generic-constraints axis: a generic without any
// interface constraint produces `specialize(%base, T)` — no
// trailing witness operand. The constrained form
// `specialize(%base, T, %witness)` is exercised by
// `specialize-with-witness-arg.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

T identity<T>(T x) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = identity<int>(a);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)(%{{[A-Za-z0-9_]+}})
