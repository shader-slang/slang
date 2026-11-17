// A `Load` of a vector or struct from a byte-address buffer is split into one load per word (WGSL,
// Metal) or per field (HLSL, CUDA, CPU). The loads whose words are not used must be removed like
// the rest of the unused value, so only the words that are read stay in the output.
// https://github.com/shader-slang/slang/issues/13739

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu -shaderobj -output-using-type

// 17 words.
struct M
{
    float4 a;
    float3 b;
    uint c;
    float d[8];
    uint e;
};

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63], stride=4):name=pool
ByteAddressBuffer pool;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint base = tid.x * 68;

    // Word 5 of the four loaded.
    uint4 q = pool.Load4(base + 16);
    outputBuffer[0] = q.y;

    // Field d[3] (word 11) of a struct that starts at word 17: word 28.
    outputBuffer[1] = asuint(pool.Load<M>(base + 68).d[3]);

    // Field c (word 7) of a struct that starts at word 39: word 46. The other fields are unused.
    M m = pool.Load<M>(base + 156);
    outputBuffer[2] = m.c;
}

// BUF: 5
// BUF-NEXT: 28
// BUF-NEXT: 46

// Each read loads one word (or field), and nothing else is loaded.

// WGSL-LABEL: fn computeMain
// WGSL-NOT: pool_0[
// WGSL: pool_0[(base_0 + u32(16) + u32(4))/4]
// WGSL-NOT: pool_0[
// WGSL: pool_0[(base_0 + u32(68) + u32(44))/4]
// WGSL-NOT: pool_0[
// WGSL: pool_0[(base_0 + u32(156) + u32(28))/4]
// WGSL-NOT: pool_0[

// METAL-LABEL: void computeMain
// METAL-NOT: pool_1[
// METAL: pool_1[(base_0 + 16U + 4U)>>2]
// METAL-NOT: pool_1[
// METAL: pool_1[(base_0 + 68U + 44U)>>2]
// METAL-NOT: pool_1[
// METAL: pool_1[(base_0 + 156U + 28U)>>2]
// METAL-NOT: pool_1[

// HLSL-LABEL: void computeMain
// HLSL-NOT: pool_0
// HLSL: pool_0.Load4(base_0 + 16U).y
// HLSL-NOT: pool_0
// HLSL: (pool_0).Load<float >(base_0 + 68U + 44U)
// HLSL-NOT: pool_0
// HLSL: pool_0.Load(base_0 + 156U + 28U)
// HLSL-NOT: pool_0

// CUDA-LABEL: void computeMain
// CUDA-NOT: pool_0
// CUDA: (globalParams_0->pool_0).Load<uint >(base_0 + 16U + 4U)
// CUDA-NOT: pool_0
// CUDA: (globalParams_0->pool_0).Load<float >(base_0 + 68U + 44U)
// CUDA-NOT: pool_0
// CUDA: (globalParams_0->pool_0).Load<uint >(base_0 + 156U + 28U)
// CUDA-NOT: pool_0
