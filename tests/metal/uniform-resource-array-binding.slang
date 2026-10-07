// A top-level `uniform` fixed-size array of textures or samplers binds to
// consecutive Metal slots from a base index, so its kernel argument must carry
// the base `[[texture(N)]]` / `[[sampler(N)]]` attribute. This also holds when
// the elements are `.Handle`-wrapped, matching the non-array `.Handle` case.
// Every index is pinned to the layout: `handleTexs` starts at 4 because `texs`
// takes four slots, and the `register(t16)` / `register(s12)` arrays check
// that the emitted index follows an explicit binding.
//
// Arrays of buffers are excluded: MSL has no direct kernel-argument form for
// them (MSL 4.1 section 2.12.1), so they need argument-buffer routing (#12291).

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
// Metal's compiler must accept the attributed array signatures end-to-end.
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

uniform Texture2D texs[4];
uniform SamplerState samps[4];
uniform Texture2D.Handle handleTexs[4];
uniform Texture2D offsetTexs[4] : register(t16);
uniform SamplerState offsetSamps[4] : register(s12);
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x % 4;
    output[tid.x] = texs[i].SampleLevel(samps[i], float2(0, 0), 0).x +
                    handleTexs[i].SampleLevel(samps[i], float2(0, 0), 0).x +
                    offsetTexs[i].SampleLevel(offsetSamps[i], float2(0, 0), 0).x;
}

// MTL-DAG: {{ texs_[0-9]+}} {{\[\[}}texture(0)]]
// MTL-DAG: {{ samps_[0-9]+}} {{\[\[}}sampler(0)]]
// MTL-DAG: {{handleTexs_[0-9]+}} {{\[\[}}texture(4)]]
// MTL-DAG: {{offsetTexs_[0-9]+}} {{\[\[}}texture(16)]]
// MTL-DAG: {{offsetSamps_[0-9]+}} {{\[\[}}sampler(12)]]

// LIB: define void @computeMain
