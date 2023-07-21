//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_6 -entry main -stage compute

// Test that RWTexture2DMS accepts an optional sampleCount argument
// and the argument correctly appears in the output hlsl.

// CHECK: RWTexture2DMS<uint > t_0 : register(u0);
// CHECK: RWTexture2DMS<uint, 4 > tExplicit_0 : register(u1);
// CHECK: TextureCubeMS<float4, 3 > tCube_0 : register(t0);

RWTexture2DMS<uint> t;
RWTexture2DMS<uint, 4> tExplicit;
TextureCubeMS<float4, 3> tCube;

SamplerState s;
RWBuffer<uint4> b;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let v = t.Load(int2(0, 0), 1) + tExplicit.Load(int2(1,1),2) + int(tCube.Load(int3(0,0,0), 3).x);
    b[tid.x] = v;
}
