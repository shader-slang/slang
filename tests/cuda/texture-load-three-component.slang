// CUDA's integer texel fetch (tex*fetch_int in the CUDA prelude) had no three-component
// specialization, so Load of a three-component texel type referenced an undefined template:
// NVRTC rejected it ("extern inline function ... was referenced but not defined") and nvcc left
// an unresolved .extern .func. The three-component specializations fetch four components and
// keep the first three, like the sample path (#12634).
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Texture2D<float3> t2;
Texture3D<int3> t3;
Texture1DArray<uint3> t1a;
Texture2DArray<float3> t2a;
RWStructuredBuffer<float3> outputBuffer;

// The two PTX-NOT lines cover the output before and after the texture fetches.
// PTX-NOT: .extern .func
// PTX-DAG: tex.level.2d.v4.f32.s32
// PTX-DAG: tex.level.3d.v4.s32.s32
// PTX-DAG: tex.level.a1d.v4.u32.s32
// PTX-DAG: tex.level.a2d.v4.f32.s32
// PTX-NOT: .extern .func
[numthreads(1, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    int i = tid.x;
    outputBuffer[0] = t2.Load(int3(i, 1, 0));
    outputBuffer[1] = float3(t3.Load(int4(i, 1, 2, 0)));
    outputBuffer[2] = float3(t1a.Load(int3(i, 1, 0)));
    outputBuffer[3] = t2a.Load(int4(i, 1, 2, 0));
}
