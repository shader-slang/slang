// An unsized `uniform` array of textures gets no `[[texture(N)]]` attribute.
// Only sized arrays bind to consecutive slots from a base index, because
// `emitFuncParamLayoutImpl` unwraps `IRArrayType` and not `IRArrayTypeBase`.
// This test fails if that unwrap starts matching unsized arrays. The emitted
// parameter is still not a valid Metal kernel argument, with or without the
// attribute, so there is no metallib directive. This pins interim behavior
// until unsized resource arrays are routed through argument buffers or
// diagnosed.

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

// `unsizedTexs` is the only texture, so no `[[texture(` may follow the kernel
// signature.
// MTL: {{\[\[}}kernel{{\]\]}} void computeMain(
// MTL-NOT: {{\[\[}}texture(
