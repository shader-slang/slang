//TEST:SIMPLE(filecheck=SPIRV_DOMAIN_TRICW):-target spirv -stage domain -entry domainMain_triangle_cw
//TEST:SIMPLE(filecheck=SPIRV_DOMAIN_ISOLINE):-target spirv -stage domain -entry domainMain_isoline_line

// SPIRV_DOMAIN_TRICW-DAG: OpExecutionMode %domainMain_triangle_cw Triangles
// SPIRV_DOMAIN_TRICW-DAG: OpExecutionMode %domainMain_triangle_cw VertexOrderCw

// SPIRV_DOMAIN_ISOLINE-NOT: OutputLinesEXT
// SPIRV_DOMAIN_ISOLINE: OpExecutionMode %domainMain_isoline_line Isolines
// SPIRV_DOMAIN_ISOLINE-NOT: OutputLinesEXT

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

struct LinePatchConst
{
    float EdgeTessFactor[2] : SV_TessFactor;
};

[shader("domain")]
[domain("isoline")]
[outputtopology("line")]
float4 domainMain_isoline_line(
    LinePatchConst patchConstants,
    OutputPatch<ControlPoint, 2> patch,
    float2 uv : SV_DomainLocation)
    : SV_Position
{
    float3 p = patch[0].position * (1.0 - uv.x) + patch[1].position * uv.x;
    return float4(p, patchConstants.EdgeTessFactor[0]);
}
