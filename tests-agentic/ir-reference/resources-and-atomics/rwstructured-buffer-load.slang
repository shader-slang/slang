//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#buffer-load-and-store
//META: doc_section_digest=a5cc503aaed86633e5970cfcf6ca63262c9a68fc95f6b94e9a9eaf7fe399c9fd
//META: purpose=`RWStructuredBuffer::Load(idx)` surfaces as `rwstructuredBufferLoad(%buf, %idx)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's buffer-load-and-store table maps `RWStructuredBuffer::Load`
// to `rwstructuredBufferLoad` with a variadic operand list starting
// with the buffer and index.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int v = rwbuf.Load(idx);
    rwbuf[0] = v;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferLoad(%rwbuf, %idx)
