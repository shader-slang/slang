//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=a5cc503aaed86633e5970cfcf6ca63262c9a68fc95f6b94e9a9eaf7fe399c9fd
//META: purpose=Eight consecutive `RWStructuredBuffer::Load(i)` calls each emit a distinct `rwstructuredBufferLoad` opcode; the documented opcode shape repeats.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: many loads from the same buffer should produce many
// `rwstructuredBufferLoad` opcodes, one per source-level call.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    s += rwbuf.Load(0);
    s += rwbuf.Load(1);
    s += rwbuf.Load(2);
    s += rwbuf.Load(3);
    s += rwbuf.Load(4);
    s += rwbuf.Load(5);
    s += rwbuf.Load(6);
    s += rwbuf.Load(7);
    rwbuf[tid.x] = s;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferLoad(%rwbuf, 0 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 1 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 2 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 3 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 4 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 5 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 6 : Int)
// IR: rwstructuredBufferLoad(%rwbuf, 7 : Int)
