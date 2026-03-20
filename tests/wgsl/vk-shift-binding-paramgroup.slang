//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute -fvk-t-shift 10 0 -fvk-b-shift 20 0 -fvk-u-shift 30 0 -fvk-s-shift 40 0

// CHECK-DAG: @binding(10) @group(0){{.*}}srv
// CHECK-DAG: @binding(31) @group(0){{.*}}uav
// CHECK-DAG: @binding(42) @group(0){{.*}}samp
// CHECK-DAG: @binding(20){{.*}}@group(0){{.*}}cb
// CHECK-DAG: @binding(13) @group(0){{.*}}tex

struct Data { float4 value; };

StructuredBuffer<Data> srv : register(t0);
RWStructuredBuffer<float> uav : register(u1);
SamplerState samp : register(s2);

cbuffer cb : register(b0) { float factor; };

Texture2D tex : register(t3);

[numthreads(1,1,1)]
void computeMain()
{
    uav[0] = srv[0].value.x * factor + tex.SampleLevel(samp, float2(0), 0).x;
}
