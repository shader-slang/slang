//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#hlsl
//META: doc_section_digest=be31c035a4983e8352a963da8245a71e0675d21c0124091e0f8e223068f3565f
//META: purpose=The HLSL backend emits HLSL source text with [numthreads] and register(uN) annotations for resources.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL backend (slang-emit-hlsl.{h,cpp}) emits HLSL: the compute
// stage attribute survives as `[numthreads(...)]`, and an
// RWStructuredBuffer is annotated with the HLSL `register(uN)` form.
// The pattern uses a bare `numthreads(` substring because FileCheck's
// `[[ ]]` is a variable reference and would not match the literal
// HLSL attribute brackets.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
// CHECK: numthreads(8, 4, 2)
// CHECK: void main(uint3 tid_{{[0-9]+}} : SV_DispatchThreadID)
