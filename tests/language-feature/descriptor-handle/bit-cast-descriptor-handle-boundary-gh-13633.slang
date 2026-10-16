// The `bit_cast` casts around a `DescriptorHandle<T>` that stay legal (shader-slang/slang#13633).
// E41206 rejects only a cast into a handle; a cast out of a handle remains an ordinary same-size
// reinterpretation, so it still gets the E41202 size check. On D3D, Vulkan and WebGPU the handle is
// read as its `uint2` value, so `bit_cast<uint2>(h)` is the handle itself: no 64-bit round trip,
// which WGSL cannot express and GLSL truncates to the low word. Under `spvBindlessTextureNV` a
// texture handle is a `uint64_t` and is read at that width. Where a handle is the native resource
// (CUDA, CPU, Metal), casting the bare resource to its handle is accepted too; those lanes check
// only that the cast is accepted and compiles, not the target's spelling of it.

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
//TEST:SIMPLE(filecheck=FLOATS):-target spirv-asm -entry handleToFloats -stage compute -O0
//TEST:SIMPLE(filecheck=FLOATS-SRC):-target glsl -entry handleToFloats -stage compute -O0
//TEST:SIMPLE(filecheck=FLOATS-SRC):-target hlsl -entry handleToFloats -stage compute -O0
//TEST:SIMPLE(filecheck=NV):-target spirv-asm -entry nvHandleBits -stage compute -O0 -capability spvBindlessTextureNV
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

// A destination other than the handle's own integer type is a second, ordinary `BitCast` from
// that value.
[noinline]
float2 floatsOf(DescriptorHandle<Texture2D<float4>> h)
{
    return bit_cast<float2>(h);
}

uniform RWStructuredBuffer<float2> outputFloats;

[shader("compute")]
[numthreads(1, 1, 1)]
void handleToFloats(uint3 tid: SV_DispatchThreadID)
{
    outputFloats[tid.x] = floatsOf(handle);
}

// FLOATS: %floatsOf = OpFunction %v2float
// FLOATS: OpBitcast %v2float %h
// FLOATS-NOT: %ulong
// FLOATS-SRC: floatsOf
// FLOATS-SRC-NOT: uint64_t
// FLOATS-SRC: return

// Under `spvBindlessTextureNV` a texture handle is a `uint64_t`, so `bit_cast<uint64_t>` is the
// handle itself and `bit_cast<uint2>` reinterprets it; a buffer handle stays a `uint2`.
uniform DescriptorHandle<StructuredBuffer<uint>> bufferHandle;
uniform RWStructuredBuffer<uint64_t> outputLong;

[noinline]
uint64_t textureLong(DescriptorHandle<Texture2D<float4>> h)
{
    return bit_cast<uint64_t>(h);
}

[noinline]
uint2 textureWords(DescriptorHandle<Texture2D<float4>> h)
{
    return bit_cast<uint2>(h);
}

[noinline]
uint2 bufferWords(DescriptorHandle<StructuredBuffer<uint>> h)
{
    return bit_cast<uint2>(h);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void nvHandleBits(uint3 tid: SV_DispatchThreadID)
{
    outputLong[tid.x] = textureLong(handle);
    outputBits[tid.x] = textureWords(handle);
    outputBits[tid.x + 1] = bufferWords(bufferHandle);
}

// NV: %textureLong = OpFunction %ulong
// NV-NEXT: %h = OpFunctionParameter %ulong
// NV-NEXT: OpLabel
// NV-NEXT: OpReturnValue %h
// NV: %textureWords = OpFunction %v2uint
// NV: OpBitcast %v2uint
// NV: %bufferWords = OpFunction %v2uint
// NV-NEXT: OpFunctionParameter %v2uint
// NV-NEXT: OpLabel
// NV-NEXT: OpReturnValue

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
