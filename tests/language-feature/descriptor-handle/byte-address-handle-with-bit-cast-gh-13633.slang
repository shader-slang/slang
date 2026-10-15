// Loading or storing a `DescriptorHandle<T>` through a byte-address buffer on WGSL and Metal
// creates a `BitCast` between the handle and a `uint64_t` during legalization (shader-slang/slang#13633).
// An unrelated user `bit_cast` in the same module makes `lowerBitCast` run over that cast too; it
// must reach the emitter as the target's own bit cast, and on WGSL the handle stays a `vec2<u32>`.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute

uniform RWByteAddressBuffer buffer;
uniform RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let texture = buffer.Load<DescriptorHandle<Texture2D<float4>>>(0);
    buffer.Store<DescriptorHandle<Texture2D<float4>>>(16, texture);
    outputBuffer[tid.x] = texture.Load(int3(0)) + bit_cast<float>(tid.x);
}

// WGSL: fn computeMain
// WGSL: : vec2<u32> = bitcast<vec2<u32>>
// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
