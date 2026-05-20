//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#buffer-types
//META: doc_section_digest=c25174867f65b28674f6e50925bd9f916f322862f95fe848ae8bb96cd35ce36b
//META: purpose=An HLSLRWStructuredBufferType uniform reaches every text-emit backend; HLSL renders it as `RWStructuredBuffer`, GLSL as a `buffer` block, and CUDA emits a global of the buffer name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLRWStructuredBufferType` is the buffer-family leaf for
// `RWStructuredBuffer<T>`. The doc's "Resource and texture type
// families" note says each requires distinct IR lowering and binding
// semantics on every backend; this test observes the per-target
// surface form rather than the IR.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> g_buf;

//CHECK_HLSL: RWStructuredBuffer
//CHECK_GLSL: buffer
//CHECK_CUDA: g_buf
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_buf[tid.x] = int(tid.x);
}
