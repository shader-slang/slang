//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=Boundary probe of `rwstructuredBufferGetElementPtr`: a runtime-bounded index (thread id, not a literal) still lowers to the same opcode with an SSA-value index instead of a literal.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The bootstrap test indexes with a literal `0`. This probe exercises
// the runtime-bounded case: the index is a thread id rather than a
// literal, so the second operand of `rwstructuredBufferGetElementPtr`
// is an SSA reference, not a constant.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = a;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%buf, %{{.*}})
// IR: store(%{{.*}}, %{{.*}})
