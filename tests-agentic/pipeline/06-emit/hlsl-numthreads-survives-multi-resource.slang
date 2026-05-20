//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#hlsl
//META: doc_section_digest=be31c035a4983e8352a963da8245a71e0675d21c0124091e0f8e223068f3565f
//META: purpose=Stress: HLSL `[numthreads(...)]` and `register(uN)` shapes survive a kernel with multiple resources of mixed read/write classification (one RW buffer, one read-only buffer, one cbuffer).
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of the HLSL emit shape: combine three resource
// shapes (RWStructuredBuffer → `register(uN)`, StructuredBuffer →
// `register(tN)`, cbuffer → `register(bN)`) and verify the HLSL emit
// keeps all three together with the compute attribute.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;
cbuffer Params
{
    int offset;
}

[shader("compute")]
[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x] + offset;
}

// CHECK-DAG: StructuredBuffer<int{{.*}}> inBuf_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK-DAG: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
// CHECK-DAG: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
// CHECK: numthreads(8, 4, 2)
