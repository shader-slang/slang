//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=35d35f0532fa227b4b1f7912fe1199f00c299b757b6503b6c6e6b0c9fd5cbc35
//META: purpose=WGSL emit uses generic-vector spellings: uint3 becomes vec3<u32>, float2 becomes vec2<f32>
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL has no uintN / floatN shorthand; vector types spell out as
// vec<rank><elem-type>.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

float pickX(float2 v) { return v.x; }

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 v = float2(float(tid.x), float(tid.y));
    outBuf[tid.x] = pickX(v) + v.y;
}

// CHECK: vec3<u32>
// CHECK: vec2<f32>
