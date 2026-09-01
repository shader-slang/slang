//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=03a91c30e9eaf1766fcdfbae6c7316ecb8108a2f9e0a27218a3bbcea152c7a7f
//META: purpose=Two Sampler2D globals each split into their own <name>_texture_* / <name>_sampler_* pair with distinct bindings
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress / multi-resource edge of the combined-texture-sampler
// split: each Sampler2D produces an independent texture+sampler
// pair, and the four resulting globals receive distinct @binding
// indices.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
Sampler2D texA;
Sampler2D texB;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a = texA.SampleLevel(float2(0, 0), 0).x;
    float b = texB.SampleLevel(float2(0, 0), 0).x;
    outBuf[tid.x] = a + b;
}

// CHECK-DAG: texA_texture_{{[0-9]+}}
// CHECK-DAG: texA_sampler_{{[0-9]+}}
// CHECK-DAG: texB_texture_{{[0-9]+}}
// CHECK-DAG: texB_sampler_{{[0-9]+}}
