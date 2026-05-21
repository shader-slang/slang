//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=An `Atomic<uint64_t>.add(1)` lowers to the documented `atomicAdd` opcode with a `UInt64` result type — the 64-bit-unsigned probe of the `AtomicOperation` family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `atomicAdd` as a representative `AtomicOperation` opcode
// but does not enumerate the element-type axis. A 64-bit unsigned
// element exercises the same opcode at the upper edge of the integer-
// width axis where SPIR-V capability gating is relevant.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<Atomic<uint64_t> > buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0].add(1);
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{.*}}, 1 : UInt64, {{.*}})
