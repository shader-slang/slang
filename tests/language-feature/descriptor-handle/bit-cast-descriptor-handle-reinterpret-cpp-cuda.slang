// `reinterpret` into a `DescriptorHandle<T>` unpacks an AnyValue with a compiler-generated bit cast
// from `uint2` to the handle. On C++ and CUDA the handle is the native resource, so the emitter
// prints that cast as `slang_bit_cast`. A `countbits` call brings a `bit_cast` in from the core
// module, which makes `lowerBitCast` run over the module; it used to abort on the handle cast with
// "Unable to generate bit_cast code for the given type" (shader-slang/slang#13633). The other
// targets build the handle without a bit cast and must compile as before.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute

uniform uint2 samplerBits;
uniform uint2 textureBits;
uniform uint mask;
uniform Texture2D tex;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    SamplerState s = reinterpret<DescriptorHandle<SamplerState>>(samplerBits);
    Texture2D t = reinterpret<DescriptorHandle<Texture2D>>(textureBits);
    output[0] = (tex.SampleLevel(s, float2(0.5), 0) + t.Load(int3(0))) * countbits(mask);
}

// CPP-DAG: slang_bit_cast<SamplerState>(
// CPP-DAG: slang_bit_cast<Texture2D<
// CPP: void computeMain
// CUDA-DAG: slang_bit_cast<SamplerState>(
// CUDA-DAG: slang_bit_cast<CUtexObject>(
// CUDA: void computeMain
// SPV: OpEntryPoint
// HLSL: void computeMain
// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
