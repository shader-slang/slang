//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=7ad88f18e708a0667a9afb539fc85293a139970c3197b087b618fd0216e6e53e
//META: purpose=`lowerCombinedTextureSamplers` fires on the Metal arm, splitting a `Sampler2D` into separate `combined_texture_<N>` and `combined_sampler_<N>` kernel parameters.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Metal is in the HLSL/Metal/WGSL arm of
// `lowerCombinedTextureSamplers`: MSL has no combined texture+sampler
// object, so a Slang `Sampler2D` must arrive at emit as two independent
// parameters. The kernel samples through the combined object so both
// halves stay live, and CHECK asserts the kernel signature carries a
// `texture2d<float, access::sample>` with a `[[texture(N)]]` slot and a
// `sampler` with a `[[sampler(N)]]` slot, named with the pass's
// `combined_texture_` / `combined_sampler_` prefixes.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Sampler2D combined;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combined.SampleLevel(float2(tid.xy), 0);
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: texture2d<float, access::sample> combined_texture_{{[0-9]+}}
// CHECK-SAME: texture(
// CHECK-SAME: sampler combined_sampler_{{[0-9]+}}
// CHECK-SAME: sampler(
