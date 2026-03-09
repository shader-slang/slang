//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vs_main -stage vertex -fvk-t-shift 0 0

// Regression test for https://github.com/shader-slang/slang/issues/10441
// Verify that -fvk-t-shift with WGSL target emits @binding/@group on
// StructuredBuffer declarations.

// CHECK: @binding(0) @group(0){{.*}}vertices

struct Vertex { float4 pos; };
struct VS_Out { float4 pos; };

StructuredBuffer<Vertex> vertices : register(t0);

VS_Out vs_main(uint vid : SV_VertexID) {
    Vertex vert = vertices[vid];
    VS_Out o;
    o.pos = vert.pos;
    return o;
}
