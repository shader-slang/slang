// The `bit_cast` casts around a `DescriptorHandle<T>` that stay legal (shader-slang/slang#13633).
// E41206 rejects only a cast into a handle; a cast out of a handle remains an ordinary same-size
// reinterpretation, so it still gets the E41202 size check. On D3D, Vulkan and WebGPU the handle is
// read as its `uint2` value, so `bit_cast<uint2>(h)` is the handle itself: no 64-bit round trip,
// which WGSL cannot express and GLSL truncates to the low word. Where a handle is the native
// resource (CUDA, CPU, Metal), casting the bare resource to its handle is accepted too.

//DIAGNOSTIC_TEST:SIMPLE(diag=SIZE):-target spirv -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target hlsl -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target glsl -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target wgsl -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target metal -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target cuda -entry sizeMismatch -stage compute
//TEST:SIMPLE(filecheck=SIZE-OTHER):-target cpp -entry sizeMismatch -stage compute

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry handleToBits -stage compute -O0
//TEST:SIMPLE(filecheck=BITS):-target glsl -entry handleToBits -stage compute -O0
//TEST:SIMPLE(filecheck=BITS):-target wgsl -entry handleToBits -stage compute -O0
//TEST:SIMPLE(filecheck=BITS):-target hlsl -entry handleToBits -stage compute -O0
//TEST:SIMPLE(filecheck=NATIVE):-target cuda -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=NATIVE):-target cpp -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=NATIVE):-target metal -entry resourceToHandle -stage compute
//TEST:SIMPLE(filecheck=MISMATCH):-target cuda -entry resourceToOtherHandle -stage compute
//TEST:SIMPLE(filecheck=MISMATCH):-target cpp -entry resourceToOtherHandle -stage compute
//TEST:SIMPLE(filecheck=MISMATCH):-target metal -entry resourceToOtherHandle -stage compute

uniform DescriptorHandle<Texture2D<float4>> handle;
uniform Texture2D<float4> texture;
uniform RWStructuredBuffer<uint> outputWord;
uniform RWStructuredBuffer<uint4> outputQuad;
uniform RWStructuredBuffer<uint2> outputBits;
uniform RWStructuredBuffer<float4> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void sizeMismatch(uint3 tid: SV_DispatchThreadID)
{
    outputWord[tid.x] = bit_cast<uint>(handle);
//SIZE:                               ^ error E41202
//SIZE:                               ^ invalid to bit_cast differently sized types
    outputQuad[tid.x] = bit_cast<uint4>(handle);
//SIZE:                                ^ error E41202
//SIZE:                                ^ invalid to bit_cast differently sized types
}

// SIZE-OTHER-COUNT-2: error[E41202]
// SIZE-OTHER-NOT: error[E41202]
// SIZE-OTHER-NOT: E99997

[noinline]
uint2 bitsOf(DescriptorHandle<Texture2D<float4>> h)
{
    return bit_cast<uint2>(h);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void handleToBits(uint3 tid: SV_DispatchThreadID)
{
    outputBits[tid.x] = bitsOf(handle);
}

// SPIRV: %bitsOf = OpFunction %v2uint
// SPIRV-NOT: OpBitcast
// SPIRV: OpReturnValue %h
// BITS: bitsOf
// BITS-NOT: uint64_t
// BITS-NOT: u64
// BITS: return h

[shader("compute")]
[numthreads(1, 1, 1)]
void resourceToHandle(uint3 tid: SV_DispatchThreadID)
{
    let h = bit_cast<DescriptorHandle<Texture2D<float4>>>(texture);
    outputBuffer[tid.x] = h.Load(int3(0));
}

// NATIVE-NOT: E41206
// NATIVE: resourceToHandle

// Only the resource the handle wraps is accepted; a different resource type is still rejected.
[shader("compute")]
[numthreads(1, 1, 1)]
void resourceToOtherHandle(uint3 tid: SV_DispatchThreadID)
{
    let h = bit_cast<DescriptorHandle<RWTexture2D<float4>>>(texture);
    outputBuffer[tid.x] = h.Load(int2(0));
}

// MISMATCH-COUNT-1: error[E41206]
// MISMATCH-NOT: error[E41206]
