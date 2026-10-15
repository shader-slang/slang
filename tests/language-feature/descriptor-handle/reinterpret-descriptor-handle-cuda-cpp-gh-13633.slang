// On CUDA and CPU a `DescriptorHandle<T>` is the native resource, and AnyValue marshalling for
// `reinterpret` packs and unpacks it with a compiler-generated `BitCast` to and from `uintN`
// (shader-slang/slang#13633). Any user `bit_cast` in the module makes `lowerBitCast` run over
// those casts too; they must reach the emitter as a plain `slang_bit_cast`, for both the 8-byte
// unpack and the 16-byte pack.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

uniform StructuredBuffer<uint2> handleBits;
uniform DescriptorHandle<Texture2D<float4>> textureHandle;
uniform DescriptorHandle<StructuredBuffer<float4>> bufferHandle;
uniform RWStructuredBuffer<float4> outputBuffer;
uniform RWStructuredBuffer<uint4> outputBits;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let texture = reinterpret<DescriptorHandle<Texture2D<float4>>>(handleBits[tid.x]);
    let textureBits = reinterpret<uint2>(textureHandle);
    let bufferBits = reinterpret<uint4>(bufferHandle);
    float scale = bit_cast<float>(textureBits.x);
    outputBuffer[tid.x] = texture.Load(int3(0)) * scale;
    outputBits[tid.x] = bufferBits;
}

// CUDA-DAG: = (slang_bit_cast<uint4 >(
// CUDA-DAG: = (slang_bit_cast<uint2 >(
// CUDA-DAG: return (slang_bit_cast<CUtexObject>(
// CUDA: void computeMain
// CPP-DAG: = (slang_bit_cast<Vector<uint32_t, 4> >(
// CPP-DAG: = (slang_bit_cast<Vector<uint32_t, 2> >(
// CPP-DAG: return (slang_bit_cast<Texture2D<
// CPP: void computeMain
