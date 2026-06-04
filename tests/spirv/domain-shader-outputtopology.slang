//TEST:SIMPLE(filecheck=SPIRV_DOMAIN_TRICW):-target spirv -stage domain -entry domainMain_triangle_cw

// SPIRV_DOMAIN_TRICW-DAG: OpExecutionMode %domainMain_triangle_cw Triangles
// SPIRV_DOMAIN_TRICW-DAG: OpExecutionMode %domainMain_triangle_cw VertexOrderCw

struct ControlPoint
{
    float3 position : POSITION;
};

struct PatchConst
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

[shader("domain")]
[domain("tri")]
[outputtopology("triangle_cw")]
float4 domainMain_triangle_cw(
    PatchConst patchConstants,
    OutputPatch<ControlPoint, 3> patch,
    float3 bary : SV_DomainLocation)
    : SV_Position
{
    float3 p =
        patch[0].position * bary.x + patch[1].position * bary.y + patch[2].position * bary.z;
    return float4(p, patchConstants.InsideTessFactor);
}
