//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=Specializing with an undefined type identifier is rejected at the check stage — no specialize IR is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that a generic argument must name a resolvable type before a
// `specialize` can be built. The identifier does not resolve, so the CHECK
// pins the undefined-identifier diagnostic under the argument's caret span.
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
