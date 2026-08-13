//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=A `Texture2D<T>` and a `SamplerState` declared separately stay separate `texture2d<T, access::sample>` and `sampler` parameters with independent slot spaces.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the counterpart to the combined-sampler split: Metal natively
// has separate texture and sampler objects, so an already-separate pair
// needs no legalization and must survive untouched. CHECK matches the
// kernel signature and then asserts, on the same line, the texture type
// with its `texture(` slot and the sampler with its own `sampler(` slot --
// showing the two slot spaces are indexed independently.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(tid.xy), 0);
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: texture2d<float, access::sample> tex_{{[0-9]+}}
// CHECK-SAME: texture(0)
// CHECK-SAME: sampler samp_{{[0-9]+}}
// CHECK-SAME: sampler(0)
