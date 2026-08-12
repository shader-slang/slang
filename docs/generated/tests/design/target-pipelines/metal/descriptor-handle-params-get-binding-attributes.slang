//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#descriptorhandlet-parameter-binding-emission
//META: doc_section_digest=159d74b6f024d007c931af7765e7aacc5da41ce801b0bc15fe83aa4c2d424647
//META: purpose=`emitFuncParamLayoutImpl` unwraps `DescriptorHandle<T>` before the per-resource-kind test, so bindless buffer / texture / sampler parameters still receive `[[buffer(N)]]` / `[[texture(N)]]` / `[[sampler(N)]]`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the bindless wrapper does not cost a parameter its binding
// slot. `DescriptorHandle<T>` is laid out as if it were `T`, so without the
// unwrap the per-kind pointer/texture/sampler tests would all fall through
// and the parameter would emit with no slot annotation at all -- which the
// Apple compiler rejects. One handle of each resource kind is used so all
// three attribute paths run; CHECK-DAG asserts a buffer, a texture and a
// sampler slot marker are each present.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform DescriptorHandle<RWStructuredBuffer<float> > hbuf;
uniform DescriptorHandle<Texture2D<float4> > htex;
uniform DescriptorHandle<SamplerState> hsamp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = htex.SampleLevel(hsamp, float2(0, 0), 0).x;
    hbuf[tid.x] = 2.0f;
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: texture2d<float, access::sample> htex_{{[0-9]+}}
// CHECK-SAME: texture(0)
// CHECK-SAME: sampler hsamp_{{[0-9]+}}
// CHECK-SAME: sampler(0)
// CHECK-SAME: float device* hbuf_{{[0-9]+}}
// CHECK-SAME: buffer(0)
