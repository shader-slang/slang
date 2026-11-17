//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:38:51+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=0f39500a5c2260589167928aaa1a2c794b4b5a7ab8e224fd5cf5024f96aac3ee
//META: purpose=An array of textures or samplers (one fixed-size level) still takes `[[texture(N)]]` / `[[sampler(N)]]` with its base slot.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `emitFuncParamLayoutImpl` tests the element type of a fixed-size array
// parameter, so `Texture2D tex[3]` and `SamplerState smp[2]` each carry one
// attribute with the base slot of the array, even though the parameter type
// is an `array<...>` wrapper.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2D tex[3];
SamplerState smp[2];
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tex[1].SampleLevel(smp[1], float2(0, 0), 0).x;
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: array<texture2d<float, access::sample>, int(3)> tex_{{[0-9]+}} {{\[\[}}texture(0)]]
// CHECK-SAME: array<sampler, int(2)> smp_{{[0-9]+}} {{\[\[}}sampler(0)]]
