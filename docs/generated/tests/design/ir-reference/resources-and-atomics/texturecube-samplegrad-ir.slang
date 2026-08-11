//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=a0ab646bb8b7c3f2d7e5acb9f418e3f9850e9bd5e2a629c46fdeeeb725e4652c
//META: purpose=`TextureCube::SampleGrad(samp, uv, dx, dy)` surfaces in `main` as a `call specialize(%SampleGrad, ..., TextureShapeCubeDType, ...)(%tex, %samp, %uv, %dx, %dy)` — the explicit-gradient overload of the doc's `sampleGrad` opcode.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit-gradient sampling shape on a cube texture,
// where each gradient is three-component rather than two. At this stage
// the sample is still a call on the specialized core-module function,
// so the CHECK pins the cube shape tag together with the texture,
// sampler, and both gradient arguments.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform TextureCube<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 uv = float3(tid);
    float3 dx = float3(0.01, 0.0, 0.0);
    float3 dy = float3(0.0, 0.01, 0.0);
    outbuf[0] = tex.SampleGrad(samp, uv, dx, dy);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShapeCubeDType{{.*}})({{.*}}%tex{{.*}}%samp{{.*}}%dx{{.*}}%dy{{.*}})
