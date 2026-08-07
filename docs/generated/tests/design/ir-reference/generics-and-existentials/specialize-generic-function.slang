//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=A call to a generic function with a concrete type argument lowers to a call whose callee is a specialize(generic, T) opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that applying a generic function to a concrete type argument is
// represented as a `specialize` applied to the generic value, and that the
// result is what the call names as its callee. The CHECK pins the
// `call specialize(<generic>, Int)` spelling at the call site, which can only
// appear if lowering built a `specialize` rather than a directly-resolved
// callee.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickFirst<int>(a, a + 1);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
