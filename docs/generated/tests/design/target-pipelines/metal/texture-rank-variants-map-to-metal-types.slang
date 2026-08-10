//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Slang's texture rank variants map onto Metal's `texture1d` / `texture3d` / `texturecube` / `texture2d_array` types, all sharing one positional `[[texture(N)]]` slot space.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sweeps the texture-rank axis in one shader so the per-variant type
// spelling and the shared slot space are observed together: Metal names
// the array variant `texture2d_array` rather than by a template argument,
// and the cube variant `texturecube`. Every texture is sampled so none is
// eliminated; CHECK-DAG asserts each Metal type name and the four
// sequential texture slots.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture1D<float> t1;
Texture3D<float4> t3;
TextureCube<float4> tc;
Texture2DArray<float4> ta;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = t1.SampleLevel(samp, float(tid.x), 0)
                  + t3.SampleLevel(samp, float3(tid), 0)
                  + tc.SampleLevel(samp, float3(tid), 0)
                  + ta.SampleLevel(samp, float3(tid), 0);
}

// CHECK-DAG: texture1d<float
// CHECK-DAG: texture3d<float
// CHECK-DAG: texturecube<float
// CHECK-DAG: texture2d_array<float
// CHECK-DAG: texture(0)
// CHECK-DAG: texture(1)
// CHECK-DAG: texture(2)
// CHECK-DAG: texture(3)
