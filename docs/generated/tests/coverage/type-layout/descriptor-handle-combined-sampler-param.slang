//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `DescriptorHandle<Sampler2D>` entry-point parameter is classified as the combined TextureSampler shader-parameter kind and collapses to a single CUDA texture object.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_getShaderParameterKindForResourceType` splits `TextureType` on
// `isCombined()`, and its combined/read arm (`ShaderParameterKind::
// TextureSampler`) is only reached from a `GetDescriptorHandleLayout` override.
// `Sampler2D` is the combined texture-sampler spelling, so wrapping it in a
// `DescriptorHandle` and passing it as an entry-point parameter is what selects
// that arm. The CHECK asserts the parameter arrives as one `CUtexObject`
// rather than as a texture/sampler pair, which is the observable consequence of
// classifying it as combined.
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<float4> outp;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(DescriptorHandle<Sampler2D> combined)
{
    outp[0] = combined.SampleLevel(float2(0, 0), 0);
}

//CUDA: __global__ void main{{[A-Za-z0-9_]*}}(CUtexObject combined{{[A-Za-z0-9_]*}})
