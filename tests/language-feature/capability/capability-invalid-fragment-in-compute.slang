//TEST:SIMPLE(filecheck=CHECK): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl
// CHECK: error 36107
SamplerState samplerState;
Texture2D<float> t2D;
RWByteAddressBuffer rw;

[numthreads(1,1,1)]
void computeMain()
{
    rw.Store(0, t2D.Sample(samplerState, float2(0, 0)));
}