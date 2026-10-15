// A `bit_cast` from plain data into a `DescriptorHandle<T>` is rejected with a located error on
// every target (shader-slang/slang#13633). Depending on the target and resource kind a handle is
// a `uint2` heap index, a `uint64_t`, or the native resource, so `lowerBitCast` has no byte layout
// to rebuild one from; the `DescriptorHandle` constructor is the defined way to make one. The
// cases cover a handle nested in a struct or an array, a cast between two handle types, an array
// of resources, and a cast reached through a generic function.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target cpp -entry computeMain -stage compute

struct HandleAndIndex
{
    DescriptorHandle<Texture2D<float4>> texture;
    uint index;
};

struct Bits
{
    uint2 handle;
    uint index;
};

uniform StructuredBuffer<uint2> handleBits;
uniform StructuredBuffer<uint64_t> handleBits64;
uniform StructuredBuffer<uint4> handlePairBits;
uniform StructuredBuffer<Bits> structBits;
uniform DescriptorHandle<RWTexture2D<float4>> otherHandle;
uniform Texture2D<float4> textures[1];
uniform RWStructuredBuffer<float4> outputBuffer;

T fromBits<T>(uint2 bits)
{
    return bit_cast<T>(bits);
//CHECK:              ^ error E41206
//CHECK:              ^ cannot bit_cast 'vector<uint,2>' to 'DescriptorHandle<Texture2D>'
//CHECK:              ^ DescriptorHandle<T>(uint2)
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let a = bit_cast<DescriptorHandle<Texture2D<float4>>>(handleBits[tid.x]);
//CHECK:                                                 ^ error E41206
//CHECK:                                                 ^ cannot bit_cast 'vector<uint,2>' to 'DescriptorHandle<Texture2D>'
//CHECK:                                                 ^ DescriptorHandle<T>(uint2)
    let b = bit_cast<DescriptorHandle<Texture2D<float4>>>(handleBits64[tid.x]);
//CHECK:                                                 ^ error E41206
//CHECK:                                                 ^ cannot bit_cast 'uint64_t' to 'DescriptorHandle<Texture2D>'
//CHECK:                                                 ^ DescriptorHandle<T>(uint2)
    let c = bit_cast<HandleAndIndex>(structBits[tid.x]);
//CHECK:                            ^ error E41206
//CHECK:                            ^ cannot bit_cast 'Bits' to 'HandleAndIndex'
//CHECK:                            ^ DescriptorHandle<T>(uint2)
    let d = bit_cast<DescriptorHandle<Texture2D<float4>>[2]>(handlePairBits[tid.x]);
//CHECK:                                                    ^ error E41206
//CHECK:                                                    ^ cannot bit_cast 'vector<uint,4>' to 'array<DescriptorHandle<Texture2D>,2>'
//CHECK:                                                    ^ DescriptorHandle<T>(uint2)
    let e = bit_cast<DescriptorHandle<Texture2D<float4>>>(otherHandle);
//CHECK:                                                 ^ error E41206
//CHECK:                                                 ^ cannot bit_cast 'DescriptorHandle<
//CHECK:                                                 ^ DescriptorHandle<T>(uint2)
    let f = fromBits<DescriptorHandle<Texture2D<float4>>>(handleBits[tid.y]);
    let g = bit_cast<DescriptorHandle<Texture2D<float4>>>(textures);
//CHECK:                                                 ^ error E41206
//CHECK:                                                 ^ cannot bit_cast 'array<Texture2D,1>' to 'DescriptorHandle<Texture2D>'
//CHECK:                                                 ^ DescriptorHandle<T>(uint2)
    outputBuffer[tid.x] = a.Load(int3(0)) + b.Load(int3(0)) + c.texture.Load(int3(c.index)) +
                          d[1].Load(int3(0)) + e.Load(int3(0)) + f.Load(int3(0)) +
                          g.Load(int3(0));
}

// The remaining targets report the same seven errors and none of them aborts.
// OTHER-COUNT-7: error[E41206]
// OTHER-NOT: E99997
