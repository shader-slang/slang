// Fixed-size arrays of combined `Sampler2D` and of `SamplerComparisonState`
// also get base `[[texture(N)]]` / `[[sampler(N)]]` attributes. A combined
// array is split into a texture array and a sampler array by
// `lowerCombinedTextureSamplers`, which builds its own array layouts, so these
// reach `emitFuncParamLayoutImpl` through a different layout producer than the
// plain arrays in `uniform-resource-array-binding.slang`.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
// Metal's compiler must accept the attributed array signatures end-to-end.
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

uniform Sampler2D combined[4];
uniform Texture2D<float> depthTexs[2];
uniform SamplerComparisonState cmpSamps[2];
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x % 4;
    uint j = tid.x % 2;
    output[tid.x] = combined[i].SampleLevel(float2(0, 0), 0).x +
                    depthTexs[j].SampleCmpLevelZero(cmpSamps[j], float2(0, 0), 0.5);
}

// MTL-DAG: {{combined_texture_[0-9]+}} {{\[\[}}texture(0)]]
// MTL-DAG: {{combined_sampler_[0-9]+}} {{\[\[}}sampler(0)]]
// MTL-DAG: {{depthTexs_[0-9]+}} {{\[\[}}texture(4)]]
// MTL-DAG: {{cmpSamps_[0-9]+}} {{\[\[}}sampler(4)]]

// LIB: computeMain
