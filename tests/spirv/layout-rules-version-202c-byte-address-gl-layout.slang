// Under `-fvk-use-gl-layout` a byte-address buffer lays out fields with std430, which already
// rounds struct sizes. With `-layout-rules-version 202c` the stride between array elements follows
// the same rules as the field offsets, so `G` is 48 bytes, the same as the `ArrayStride` of a
// std430 structured buffer of `G`, and the second element's `w` is at byte 48 + 16 = 64.
//
// Without the option the stride stays the natural size of `G`, 32 bytes, as it was before the
// option existed, so the second element's `w` is read at byte 32 + 16 = 48.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-gl-layout -layout-rules-version 202c
//TEST:SIMPLE(filecheck=OLD):-target spirv-asm -emit-spirv-directly -stage compute -entry computeMain -fvk-use-gl-layout

struct S
{
    double x;
    int y;
};

struct G
{
    S z;
    float w;
    float3 f;
    float t;
};

ByteAddressBuffer bab;
RWStructuredBuffer<G> sb;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[0] = asuint(bab.Load<G[2]>(tid.x * 128)[1].w);
    output[1] = asuint(sb[tid.x].w);
}

// CHECK: OpDecorate %_runtimearr_G_std430 ArrayStride 48
// CHECK: OpIAdd %uint %{{[0-9]+}} %uint_64

// OLD: OpDecorate %_runtimearr_G_std430 ArrayStride 48
// OLD: OpIAdd %uint %{{[0-9]+}} %uint_48
