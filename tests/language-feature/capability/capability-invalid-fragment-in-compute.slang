//TEST:SIMPLE(filecheck=CHECK): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl
// CHECK: error 36107
SamplerState samplerState;
Texture2D<int> rw;

[numthreads(1,1,1)]
void computeMain()
{
    clip(rw.Load(0));
}