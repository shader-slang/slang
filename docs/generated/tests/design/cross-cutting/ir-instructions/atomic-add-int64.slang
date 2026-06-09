//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=An `Atomic<int64_t>.add(1)` lowers to the documented `atomicAdd` opcode with an `Int64` result type — 64-bit signed companion of the `atomic-add-uint64` probe.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `atomic-add-uint64.slang`: pins `atomicAdd` at the
// 64-bit signed element edge. Confirms the opcode name does not vary
// with signedness while the result type does.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<Atomic<int64_t> > buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0].add(1);
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{.*}}, 1 : Int64, {{.*}})
