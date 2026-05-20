//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#buffer-types
//META: doc_section_digest=c25174867f65b28674f6e50925bd9f916f322862f95fe848ae8bb96cd35ce36b
//META: purpose=An HLSLStructuredBufferType (read-only) is distinct from its RW variant; the HLSL emit renders it as `StructuredBuffer<T>`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLStructuredBufferType` is the read-only buffer leaf; its
// surface spelling is `StructuredBuffer<T>` and the HLSL emit
// preserves that spelling (distinct from `RWStructuredBuffer<T>`).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<int> g_in;
RWStructuredBuffer<int> g_out;

//CHECK: RWStructuredBuffer<int
//CHECK: StructuredBuffer<int
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_out[tid.x] = g_in[tid.x] + 1;
}
