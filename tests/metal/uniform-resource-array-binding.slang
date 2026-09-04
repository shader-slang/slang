// A top-level `uniform` fixed-size array of textures or samplers binds to
// consecutive Metal slots from a base index, so its kernel argument must carry
// the base `[[texture(N)]]` / `[[sampler(N)]]` attribute. This also holds when
// the elements are `.Handle`-wrapped, matching the scalar handle case.
//
// Arrays of buffers are excluded: they are not valid direct Metal kernel
// arguments and need argument-buffer routing (see #12291).

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

uniform Texture2D texs[4];
uniform SamplerState samps[4];
uniform Texture2D.Handle handleTexs[4];
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x % 4;
    output[tid.x] = texs[i].SampleLevel(samps[i], float2(0, 0), 0).x +
                    handleTexs[i].SampleLevel(samps[i], float2(0, 0), 0).x;
}

// MTL-DAG: {{texs_[0-9]+}} {{\[\[}}texture(
// MTL-DAG: {{samps_[0-9]+}} {{\[\[}}sampler(
// MTL-DAG: {{handleTexs_[0-9]+}} {{\[\[}}texture(

// LIB: computeMain
