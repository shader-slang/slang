//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-16T08:36:25Z
//META: source_commit=d3b56927b854c14fa1ac169b72da6e080ea80833
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=031d306ccbb9670357e2a8af6d7e786afcb208ea0c4a357ccf9a309fc8034205
//META: purpose=The backing pointer for a top-level ConstantBuffer<T> is loaded from SLANG_globalParams, after which reading its contents is eligible for __ldg global-memory lowering.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA stores the ConstantBuffer backing pointer in its synthesized
// `__constant__` parameter group. The pointer load stays plain, but
// dereferencing the pointed-to global memory is eligible for `__ldg`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

ConstantBuffer<uint> inBuf;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf;
}

// CHECK: __ldg(globalParams_{{[0-9]+}}->inBuf_{{[0-9]+}})
