//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=Boundary probe of `rwstructuredBufferGetElementPtr`: writing to one index and reading from another in the same buffer emits two distinct `getElementPtr` instructions, plus paired `store` and `load`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two accesses to the same buffer (one write, one read) must each
// produce their own `rwstructuredBufferGetElementPtr` paired with
// `store` / `load`. This probes that the IR does not dedupe a
// stateful-buffer pointer load.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
    buf[1] = buf[0] + 1;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%buf, 0 : Int)
// IR: store(%{{.*}}, %{{.*}})
// IR: rwstructuredBufferGetElementPtr(%buf, 1 : Int)
// IR: load(%{{.*}})
// IR: store(%{{.*}}, %{{.*}})
