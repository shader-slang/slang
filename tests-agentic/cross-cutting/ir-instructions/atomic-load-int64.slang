//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=An `Atomic<int64_t>.load()` lowers to the documented `atomicLoad` opcode of the `AtomicOperation` family on a 64-bit signed element.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc explicitly lists `atomicLoad / atomicStore / atomicAdd / ...`
// as the `AtomicOperation` family. The `atomicLoad` read sibling on a
// 64-bit signed element pins both the opcode name and the element-type
// width axis.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<Atomic<int64_t> > in_buf;
uniform RWStructuredBuffer<int64_t> out_buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    out_buf[0] = in_buf[0].load();
}

// IR-LABEL: func %main
// IR: atomicLoad(%{{.*}}, {{.*}})
