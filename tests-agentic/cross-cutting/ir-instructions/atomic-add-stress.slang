//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=Stress probe of the documented `atomicAdd` family: 64 threads each calling `.add(1)` on a shared `Atomic<int>` produce an `atomicAdd` instruction in the per-thread IR body.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `atomicAdd` as a representative `AtomicOperation`
// family opcode. A high static thread count (64) writing to a shared
// `Atomic<int>` is a stress configuration for that opcode at the
// lowering layer — every thread emits the same IR-level
// `atomicAdd(...)` against the same buffer slot.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<Atomic<int> > buf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0].add(1);
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{.*}}, 1 : Int, {{.*}})
