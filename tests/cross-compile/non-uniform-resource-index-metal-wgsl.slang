// NonUniformResourceIndex compiles for Metal and WGSL, and the emitter drops the call: neither
// target has a non-uniform annotation. See the note at the top of
// source/slang/slang-ir-float-non-uniform-resource-index.cpp for when that is correct.

// The runs check the emitted text only, not downstream validity: core WGSL has no arrays of
// textures, and Tint rejects the `array<texture_2d<f32>, ...>` declarations checked below (#9528).
// Those checks pin today's spelling and will change when binding arrays are emitted.
//
// Besides the plain index, the kernel covers a wrapper around a cast index, nested wrappers, and
// a wrapped index whose indexed resource is passed to a function. The emitter drops the wrapper
// wherever it ends up, so these runs show that the Textual-mode floatNonUniformResourceIndex pass
// handles those shapes and that no call is left, not where the pass moved the wrapper.
//
// Run 1 (Metal) and run 2 (WGSL) index a sized array.
// Run 3 indexes an unsized array on WGSL, emitted as a runtime-sized `array<texture_2d<f32>>`.
// Metal is left out because an unsized resource array is not yet a valid Metal kernel argument
// (see tests/metal/uniform-unsized-resource-array-unbound.slang).

//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL_UNBOUNDED): -target wgsl -stage compute -entry computeMain -DUNBOUNDED
// Runs 4-6 repeat runs 1-3 and only check that no `NonUniformResourceIndex` is left anywhere in
// the output: in the checks above, a `{{.*}}` could match across a call that was not dropped.
//TEST:SIMPLE(filecheck=NOCALL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCALL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCALL): -target wgsl -stage compute -entry computeMain -DUNBOUNDED
// Runs 7 and 8 index a buffer, which is valid WGSL today, so Tint also compiles it.
//TEST:SIMPLE(filecheck=BUFFER): -target wgsl -stage compute -entry bufferMain
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -stage compute -entry bufferMain

// NOCALL-NOT: NonUniformResourceIndex
// NOCALL: computeMain
// NOCALL-NOT: NonUniformResourceIndex

#ifdef UNBOUNDED
Texture2D textures[];
#else
Texture2D textures[8];
#endif
SamplerState linearSampler;
StructuredBuffer<uint> materialIndices;
RWStructuredBuffer<float4> outputBuffer;

float4 sampleTexture(Texture2D texture)
{
    return texture.SampleLevel(linearSampler, float2(0.25, 0.25), 0.0);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint index = NonUniformResourceIndex(materialIndices[dispatchThreadID.x]);
    float4 result = textures[index].SampleLevel(linearSampler, float2(0.5, 0.5), 0.0);
    result += textures[NonUniformResourceIndex(uint(float(dispatchThreadID.y)))].SampleLevel(
        linearSampler,
        float2(0.5, 0.5),
        1.0);
    result += textures[NonUniformResourceIndex(NonUniformResourceIndex(dispatchThreadID.z))]
                  .SampleLevel(linearSampler, float2(0.5, 0.5), 2.0);
    result += sampleTexture(textures[NonUniformResourceIndex(materialIndices[dispatchThreadID.y])]);
    outputBuffer[dispatchThreadID.x] = result;
}

// METAL-NOT: NonUniformResourceIndex
// METAL: textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[{{.*}}.sample({{.*}}level((0.0f))
// METAL: textures_{{[0-9]+}}[uint(float({{.*}}.sample({{.*}}level((1.0f))
// METAL: textures_{{[0-9]+}}[{{[A-Za-z0-9_.]+}}]{{.*}}.sample({{.*}}level((2.0f))
// METAL: sampleTexture_{{[0-9]+}}({{.*}}textures_{{[0-9]+}}[{{.*}}materialIndices_{{[0-9]+}}[
// METAL-NOT: NonUniformResourceIndex

// WGSL-NOT: NonUniformResourceIndex
// WGSL: textures_{{[0-9]+}} : array<texture_2d<f32>, i32(8)>;
// WGSL: textureSampleLevel((textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[{{.*}}(0.0f)
// WGSL: textureSampleLevel((textures_{{[0-9]+}}[u32(f32({{.*}}(1.0f)
// WGSL: textureSampleLevel((textures_{{[0-9]+}}[{{[A-Za-z0-9_.]+}}]{{.*}}(2.0f)
// WGSL: sampleTexture_{{[0-9]+}}(textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// WGSL-NOT: NonUniformResourceIndex

// WGSL_UNBOUNDED-NOT: NonUniformResourceIndex
// WGSL_UNBOUNDED: textures_{{[0-9]+}} : array<texture_2d<f32>>;
// WGSL_UNBOUNDED: textureSampleLevel((textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[{{.*}}(0.0f)
// WGSL_UNBOUNDED: textureSampleLevel((textures_{{[0-9]+}}[u32(f32({{.*}}(1.0f)
// WGSL_UNBOUNDED: textureSampleLevel((textures_{{[0-9]+}}[{{[A-Za-z0-9_.]+}}]{{.*}}(2.0f)
// WGSL_UNBOUNDED: sampleTexture_{{[0-9]+}}(textures_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// WGSL_UNBOUNDED-NOT: NonUniformResourceIndex

[numthreads(64, 1, 1)]
void bufferMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[NonUniformResourceIndex(materialIndices[dispatchThreadID.x])] = float4(1.0);
}

// BUFFER-NOT: NonUniformResourceIndex
// BUFFER: outputBuffer_{{[0-9]+}}[materialIndices_{{[0-9]+}}[
// BUFFER-NOT: NonUniformResourceIndex

// TINT: OpEntryPoint
