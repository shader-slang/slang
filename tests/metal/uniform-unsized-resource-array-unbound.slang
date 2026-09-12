// An unsized `uniform` array of resources is a bindless / argument-buffer case,
// not a valid direct Metal kernel argument, so it must NOT receive a base
// `[[texture(N)]]` binding attribute — only sized arrays do (#12291). This is a
// textual-emit check only (no metallib): the unbound param is not valid MSL to
// compile end-to-end, which is exactly why it is excluded from direct binding.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain

uniform Texture2D unsizedTexs[];
uniform SamplerState samp;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = unsizedTexs[tid.x].SampleLevel(samp, float2(0, 0), 0).x;
}

// MTL: {{unsizedTexs_[0-9]+}}[]
// MTL-NOT: {{unsizedTexs_[0-9]+}}[] {{\[\[}}texture(
