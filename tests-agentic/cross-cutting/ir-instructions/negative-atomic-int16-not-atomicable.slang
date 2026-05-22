//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=`Atomic<int16_t>` is rejected at type-check time because `int16_t` does not conform to the `IAtomicable` interface — bounds the AtomicOperation element-type axis below 32 bits on the integer side.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names the `AtomicOperation` family without enumerating the
// admissible element types. This negative test pins the lower edge of
// the integer-width axis: `int16_t` (and by symmetry `int8_t`/`uint16_t`/
// `uint8_t`) does not satisfy the `IAtomicable` interface required of
// the `Atomic<T>` type parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm

uniform RWStructuredBuffer<Atomic<int16_t> > buf;
//CHECK:                   ^^^^^^ type argument doesn't conform to interface
// CHECK: E38021
// CHECK: E38029

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0].add(1);
}
