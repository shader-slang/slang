//TEST:SIMPLE(filecheck=CHECK): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl
// CHECK: error 36107
SamplerState samplerState;
RWByteAddressBuffer rw;

[numthreads(1,1,1)]
void computeMain()
{
    clip(-1);
    rw.Store(0, 1);
}