//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vs_main -stage vertex -fvk-t-shift 10 0 -fvk-s-shift 20 0

// Regression test for https://github.com/shader-slang/slang/issues/10441
// Verify that -fvk-*-shift with WGSL target emits @binding/@group on
// resource declarations of various kinds (ShaderResource, SamplerState).

// CHECK-DAG: @binding(10) @group(0){{.*}}vertices
// CHECK-DAG: @binding(11) @group(0){{.*}}myTexture
// CHECK-DAG: @binding(20) @group(0){{.*}}mySampler

struct Vertex { float4 pos; };
struct VS_Out { float4 pos; };

StructuredBuffer<Vertex> vertices : register(t0);
Texture2D<float4> myTexture : register(t1);
SamplerState mySampler : register(s0);

VS_Out vs_main(uint vid : SV_VertexID) {
    Vertex vert = vertices[vid];
    float4 sampled = myTexture.SampleLevel(mySampler, float2(0, 0), 0);
    VS_Out o;
    o.pos = vert.pos + sampled;
    return o;
}
