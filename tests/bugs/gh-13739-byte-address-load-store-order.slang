// Unused byte-address loads can now be removed (#13739), and a load with one use can be written
// inside that use. A load must still read its words before a later store to the same words, even
// when only some of the loaded words are used.
// https://github.com/shader-slang/slang/issues/13739

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -wgpu -shaderobj -output-using-type

struct S
{
    uint a;
    uint2 b;
    uint c;
};

//TEST_INPUT:ubuffer(data=[10 11 12 13 20 21 22 23], stride=4):name=rw
RWByteAddressBuffer rw;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint base = tid.x * 32;

    // Words 0-3 are loaded, then word 1 is overwritten; the loaded value is the old one.
    uint4 v = rw.Load4(base);
    rw.Store(base + 4, 100u);
    outputBuffer[0] = v.y;
    outputBuffer[1] = rw.Load(base + 4);

    // A struct at words 4-7; only field c (word 7) is used, after word 7 is overwritten.
    S s = rw.Load<S>(base + 16);
    rw.Store(base + 28, 200u);
    outputBuffer[2] = s.c;
    outputBuffer[3] = rw.Load(base + 28);
}

// BUF: 11
// BUF-NEXT: 100
// BUF-NEXT: 23
// BUF-NEXT: 200

// WGSL-LABEL: fn computeMain
// WGSL: [[V:[A-Za-z0-9_]+]] : u32 = rw_0[
// WGSL: rw_0[{{.*}}] = u32(100);
// WGSL: outputBuffer_0[i32(0)] = [[V]];
// WGSL: [[C:[A-Za-z0-9_]+]] : u32 = rw_0[(base_0 + u32(16) + u32(12))/4];
// WGSL: rw_0[{{.*}}] = u32(200);
// WGSL: outputBuffer_0[i32(2)] = [[C]];

// METAL-LABEL: void computeMain
// METAL: uint [[V:[A-Za-z0-9_]+]] = as_type<uint>(rw_1[
// METAL: rw_1[{{.*}}] = as_type<uint32_t>(100U);
// METAL: outputBuffer_0+int(0)) = [[V]];
// METAL: uint [[C:[A-Za-z0-9_]+]] = as_type<uint>({{.*}}rw_0[(base_0 + 16U + 12U)>>2]);
// METAL: rw_0[{{.*}}] = as_type<uint32_t>(200U);
// METAL: outputBuffer_0+int(2)) = [[C]];

// HLSL-LABEL: void computeMain
// HLSL: uint4 [[V:[A-Za-z0-9_]+]] = rw_0.Load4(base_0);
// HLSL: rw_0.Store({{.*}}, 100U);
// HLSL: outputBuffer_0[int(0)] = [[V]].y;
// HLSL: uint [[C:[A-Za-z0-9_]+]] = rw_0.Load(base_0 + 16U + 12U);
// HLSL: rw_0.Store({{.*}}, 200U);
// HLSL: outputBuffer_0[int(2)] = [[C]];
