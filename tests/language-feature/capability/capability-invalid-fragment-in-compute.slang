//TEST:SIMPLE(filecheck=CHECK): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl -DPRE
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl -DPOST
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl -ignore-capabilities -DPRE
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target hlsl -emit-spirv-directly -entry computeMain -stage compute -allow-glsl -ignore-capabilities -DPOST

// CHECK_IGNORE_CAPS-NOT: error 36107
// CHECK: error 36107
SamplerState samplerState;
Texture2D<int> rw;

[numthreads(1,1,1)]
void computeMain()
{
#ifdef PRE
    rw.Load(0);
#endif
    clip(0.0f);
#ifdef POST
    rw.Load(0);
#endif
}