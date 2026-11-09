//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=HLSL emit spells vectors in HLSL form (`float3`/`float4`) — not GLSL's `vec3` and not WGSL's `vec3<f32>`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSLSourceEmitter overrides the C-like base to spell
// fixed-width vectors using the HLSL `float<N>` convention.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float3(float(tid.x), 0.0, 1.0);
}

// CHECK: RWStructuredBuffer<float3{{.*}}> outBuf_{{[0-9]+}}
// CHECK: float3(
