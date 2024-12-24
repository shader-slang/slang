//TEST:SIMPLE(filecheck=CHECK): -target wgsl
//CHECK: @location(0)
//CHECK: @location(1)

struct VertexOutput {
    float4 clipPos : SV_POSITION;
    float3 color;
}

[shader("vertex")]
func vertexMain(float3 pos, float3 color)->VertexOutput {
    VertexOutput out;
    out.clipPos = float4(pos, 1.0);
    out.color = color;
    return out;
}
