// Texture Load with a texel offset on CUDA. The texNDfetch functions take no offset, so the offset is
// added to the texel coordinate (not to the array layer or the mip level).
//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<float4> outputBuffer;
Texture1D<float4> t1D;
Texture2D<float4> t2D;
Texture2DArray<float4> t2DArray;
Texture3D<float4> t3D;

// CHECK: [[C1:coord_[0-9]+]] = make_int2 (i_{{[0-9]+}}, int(0));
// CHECK: [[C1]])->x) = {{.*}}[[C1]])->x) + int(1);
// CHECK: tex1Dfetch_int<float4 >(
// CHECK: [[C2:coord_[0-9]+]] = make_int3 (i_{{[0-9]+}}, int(1), int(2));
// CHECK: [[C2]])->x) = {{.*}}[[C2]])->x) + int(1);
// CHECK: [[C2]])->y) = {{.*}}[[C2]])->y) + int(-1);
// CHECK: tex2Dfetch_int<float4 >(
// CHECK: [[C3:coord_[0-9]+]] = make_int4 (i_{{[0-9]+}}, int(1), int(1), int(0));
// CHECK: [[C3]])->x) = {{.*}}[[C3]])->x) + int(1);
// CHECK: tex2DArrayfetch_int<float4 >(
// CHECK: [[C4:coord_[0-9]+]] = make_int4 (i_{{[0-9]+}}, int(1), int(2), int(1));
// CHECK: [[C4]])->x) = {{.*}}[[C4]])->x) + int(1);
// CHECK: [[C4]])->z) = {{.*}}[[C4]])->z) + int(-1);
// CHECK: tex3Dfetch_int<float4 >(
[numthreads(4, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    int i = tid.x;
    float4 r = t1D.Load(int2(i, 0), 1);
    r += t2D.Load(int3(i, 1, 2), int2(1, -1));
    r += t2DArray.Load(int4(i, 1, 1, 0), int2(1, 0));
    r += t3D.Load(int4(i, 1, 2, 1), int3(1, 0, -1));
    outputBuffer[i] = r;
}
