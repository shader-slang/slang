// A `bit_cast` whose destination is or contains a `DescriptorHandle<T>` is rejected with a located
// E41206 on every target (shader-slang/slang#13633), because the handle's representation depends
// on the target. The cases cover integer sources, a handle nested in a struct or an array, a cast
// between two handle types, an array of resources, and a cast reached through a generic function.
// The casts that stay legal are in bit-cast-descriptor-handle-boundary-gh-13633.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target cpp -entry computeMain -stage compute

// Where a handle is a heap index rather than the resource, casting the resource itself is also
// rejected (on CUDA, CPU and Metal it is accepted; see the boundary test).
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE):-target spirv -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=RESOURCE-OTHER):-target hlsl -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=RESOURCE-OTHER):-target glsl -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=RESOURCE-OTHER):-target wgsl -entry resourceToHandle -stage compute

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
uniform Texture2D<float4> texture;

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

// The remaining targets report exactly the same seven errors and none of them aborts.
// OTHER-COUNT-7: error[E41206]
// OTHER-NOT: error[E41206]
// OTHER-NOT: E99997

[shader("compute")]
[numthreads(1, 1, 1)]
void resourceToHandle(uint3 tid: SV_DispatchThreadID)
{
    let h = bit_cast<DescriptorHandle<Texture2D<float4>>>(texture);
//RESOURCE:                                              ^ error E41206
//RESOURCE:                                              ^ cannot bit_cast 'Texture2D' to 'DescriptorHandle<Texture2D>'
//RESOURCE:                                              ^ DescriptorHandle<T>(uint2)
    outputBuffer[tid.x] = h.Load(int3(0));
}

// RESOURCE-OTHER-COUNT-1: error[E41206]
// RESOURCE-OTHER-NOT: error
