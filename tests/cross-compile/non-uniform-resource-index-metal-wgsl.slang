// NonUniformResourceIndex compiles for Metal and WGSL, and the emitter drops the call: neither
// target has a non-uniform annotation. See the note at the top of
// source/slang/slang-ir-float-non-uniform-resource-index.cpp for when that is correct.

// Run 1 (Metal) and run 2 (WGSL) index a sized array.
// Run 3 indexes an unsized array on WGSL, emitted as a runtime-sized `array<texture_2d<f32>>`.
// Metal is left out because an unsized resource array is not yet a valid Metal kernel argument
// (see tests/metal/uniform-unsized-resource-array-unbound.slang).

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL_UNBOUNDED): -target wgsl -stage compute -entry computeMain -DUNBOUNDED

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
// METAL-NOT: NonUniformResourceIndex

// WGSL-NOT: NonUniformResourceIndex
// WGSL: textures_{{[0-9]+}} : array<texture_2d<f32>, i32(8)>;
// WGSL: textureSampleLevel((textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// WGSL-NOT: NonUniformResourceIndex

// WGSL_UNBOUNDED-NOT: NonUniformResourceIndex
// WGSL_UNBOUNDED: textures_{{[0-9]+}} : array<texture_2d<f32>>;
// WGSL_UNBOUNDED: textureSampleLevel((textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// WGSL_UNBOUNDED-NOT: NonUniformResourceIndex
