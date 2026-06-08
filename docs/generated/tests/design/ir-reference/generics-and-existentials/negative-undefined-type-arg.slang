//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=Specializing with an undefined type identifier is rejected at the check stage — no specialize IR is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative on the `specialize` argument-validity axis: passing
// a name that doesn't resolve as the type argument prevents the
// `arg` operand of `specialize(base, arg, ...)` (per
// `### specialize`) from being constructed. The compiler must
// reject before lowering.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

T identity<T>(T x) { return x; }

uniform RWStructuredBuffer<int> buf;
uniform int a;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = identity<UndefinedType>(a);
//CHECK:              ^^^^^^^^^^^^^ undefined identifier 'UndefinedType'.
// CHECK: E30015
}
