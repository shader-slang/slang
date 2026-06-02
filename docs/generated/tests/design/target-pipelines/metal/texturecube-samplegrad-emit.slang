//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=TextureCube SampleGrad lowers to the Metal `.sample(samp, dir, gradientcube(dPdx, dPdy))` call shape.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sampling intrinsics on TextureCube map to Metal's .sample
// method. For SampleGrad, the explicit gradients are wrapped in
// the cube-specific `gradientcube(...)` selector at emit time.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

TextureCube<float4> texCube;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = texCube.SampleGrad(
        samp,
        float3(tid),
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0));
}

// CHECK: texturecube<float
// CHECK: .sample(
// CHECK: gradientcube(
