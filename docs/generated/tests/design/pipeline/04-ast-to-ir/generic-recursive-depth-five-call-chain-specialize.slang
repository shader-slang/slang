//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d11d731c1f470905e85d4dccd140c912ec50e6d88afbda6ff67159fa80cd78b2
//META: purpose=A five-deep chain of generic-from-generic calls lowers to five `IRGeneric` wrappers; each generic body holds an unspecialized `call specialize(...)` to the inner generic — confirming that specialization is deferred recursively.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: generic recursive-instantiation depth axis (depth 5).
// Each generic wrapper calls the next via
// `call specialize(%inner, %T)` — none of the bodies are inlined or
// specialized in the lowering output. We expect to see at least
// five `call specialize(` lines (depth-5 chain + outer call from
// `main`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T g1<T>(T x) { return x; }
T g2<T>(T x) { return g1<T>(x); }
T g3<T>(T x) { return g2<T>(x); }
T g4<T>(T x) { return g3<T>(x); }
T g5<T>(T x) { return g4<T>(x); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = g5<int>(a);
}

// CHECK: call specialize(
// CHECK: call specialize(
// CHECK: call specialize(
// CHECK: call specialize(
// CHECK: call specialize(
