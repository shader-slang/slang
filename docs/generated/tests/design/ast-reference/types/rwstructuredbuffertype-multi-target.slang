//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=An HLSLRWStructuredBufferType uniform reaches every text-emit backend; each backend lowers it to its documented surface form (HLSL RWStructuredBuffer, GLSL buffer block, Metal device pointer, WGSL storage, CUDA global).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies HLSLRWStructuredBufferType is the buffer-family leaf for
// `RWStructuredBuffer<T>` and lowers to a distinct surface form per
// backend. This is the bundle's canonical multi-target type-lowering
// observation. Each per-target CHECK matches that backend's spelling.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> g_buf;

//CHECK_HLSL: RWStructuredBuffer
//CHECK_GLSL: buffer
//CHECK_SPV: OpTypeRuntimeArray
//CHECK_METAL: device
//CHECK_WGSL: storage
//CHECK_CUDA: g_buf
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_buf[tid.x] = int(tid.x);
}
