//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#floatnonuniformresourceindex
//META: doc_section_digest=ffbb0e6bd6bd50f592bc8662e57b5222bd859b8745c69c1b89379aaec65514af
//META: purpose=A NonUniformResourceIndex(...) call on WGSL is accepted (not capability-rejected) and the base emitter drops the wrapper, leaving the plain index into a binding_array
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states WGSL has no non-uniform annotation, so the wrapper reduces
// to its operand; a texture array element emits through binding_array.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2D<float4> texs[4];
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = texs[NonUniformResourceIndex(tid.x)].SampleLevel(samp, float2(0.5, 0.5), 0);
}

// CHECK: var texs_0 : binding_array<texture_2d<f32>, i32(4)>;
// CHECK: textureSampleLevel((texs_0[
// CHECK-NOT: NonUniform
