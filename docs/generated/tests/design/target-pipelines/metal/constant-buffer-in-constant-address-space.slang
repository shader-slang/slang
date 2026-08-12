//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=794e6b917e6a05e1bb5a790707b746f03faed4ff03cdce751a3d870c7ccc928f
//META: purpose=A `ConstantBuffer<T>` emits as a `T constant*` pointer parameter with a positional `[[buffer(N)]]` slot.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bindful half of Metal's parameter model: unlike a
// `ParameterBlock`, a constant buffer keeps a real binding slot and lands
// in the `constant` address space. The kernel reads two fields of the
// buffer so it stays live; CHECK matches the kernel signature and then, on
// the same emitted line, the `constant*` parameter and its `buffer(` slot
// marker.

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
