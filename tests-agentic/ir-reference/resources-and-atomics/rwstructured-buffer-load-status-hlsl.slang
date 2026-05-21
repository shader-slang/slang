//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#structuredbufferloadstatus
//META: doc_section_digest=a5cc503aaed86633e5970cfcf6ca63262c9a68fc95f6b94e9a9eaf7fe399c9fd
//META: purpose=`RWStructuredBuffer<T>::Load(idx, out status)` on `-target hlsl` surfaces the documented `rwstructuredBufferLoadStatus` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's buffer-load-and-store table lists
// `rwstructuredBufferLoadStatus` (variadic, `min=3`) as the IR form
// of the `RWStructuredBuffer::Load(idx, out status)` overload. HLSL
// is the natural target where the status path is supported.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main -profile cs_6_6

uniform RWStructuredBuffer<int> rwbuf;
uniform RWStructuredBuffer<int> sink;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    uint status;
    int v = rwbuf.Load(idx, status);
    sink[idx] = v + (int)status;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferLoadStatus
