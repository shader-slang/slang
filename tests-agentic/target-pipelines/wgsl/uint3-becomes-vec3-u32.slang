//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
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
