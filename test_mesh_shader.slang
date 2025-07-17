struct MeshPayload
{
    float4 pos;
};

struct PerVertex {
    float4 pos : SV_Position;
};

struct PerPrimitive {
    uint32_t primitive_id : SV_PrimitiveID;
};

[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main(
    in payload MeshPayload meshPayload,
    out vertices PerVertex vertices[3],
    out indices uint3 triangles[1],
    out primitives PerPrimitive primitive_data[1])
{
    SetMeshOutputCounts(3, 1);
    
    vertices[0].pos = vertices[1].pos = vertices[2].pos = meshPayload.pos;
    triangles[0] = uint3(0, 1, 2);
    primitive_data[0].primitive_id = 0;
}