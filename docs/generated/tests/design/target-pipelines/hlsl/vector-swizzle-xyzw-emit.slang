//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
//META: purpose=Expansion: a vector swizzle (.xy) survives the pipeline as `.xy` in HLSL emit text.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL uses the same `.xyzw` swizzle spelling as GLSL/WGSL. The
// emitter passes the swizzle name through unchanged.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    float4 v;
};
RWStructuredBuffer<float2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = v.xy;
}

// CHECK: .xy
