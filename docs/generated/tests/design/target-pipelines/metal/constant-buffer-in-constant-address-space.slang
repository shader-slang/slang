//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=ConstantBuffer<T> emits as a wrapper-struct pointer in Metal's `constant` address space with a positional [[buffer(N)]] attribute.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ConstantBuffer<Params>` lowers to a wrapped struct passed as
// `Params_0 constant*` (Metal's constant address space) with a
// positional `[[buffer(N)]]` slot. The cbuffer element wrapping
// is `wrapCBufferElementsForMetal`'s job.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Params
{
    float4 v;
    uint n;
};
ConstantBuffer<Params> P;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = P.v * float(P.n);
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: Params_{{[0-9]+}} constant* P_{{[0-9]+}}
// CHECK-SAME: buffer(
