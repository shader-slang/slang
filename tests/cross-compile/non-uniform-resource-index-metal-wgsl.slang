// NonUniformResourceIndex compiles for Metal and WGSL and returns its argument unchanged:
// neither target has a non-uniform qualifier. Metal handles non-uniform indices into resource
// arrays itself, and the WebGPU binding-array proposals allow non-uniform indexing without an
// annotation.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -DUNBOUNDED

#ifdef UNBOUNDED
Texture2D textures[];
#else
Texture2D textures[8];
#endif
SamplerState linearSampler;
StructuredBuffer<uint> materialIndices;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = NonUniformResourceIndex(materialIndices[dispatchThreadID.x]);
    outputBuffer[dispatchThreadID.x] =
        textures[index].SampleLevel(linearSampler, float2(0.5, 0.5), 0.0);
}

// METAL-NOT: NonUniformResourceIndex
// METAL: textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// METAL-SAME: .sample(

// WGSL-NOT: NonUniformResourceIndex
// WGSL: textureSampleLevel((textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
