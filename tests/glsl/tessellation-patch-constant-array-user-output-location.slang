//TEST:SIMPLE(filecheck=HULL):  -target glsl -entry hullMain  -stage hull   -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain -allow-glsl

// Hull shader: an arrayed patch user output (TANGENT) spans one location per
// element, so the following user semantic must start past the array's span.
//HULL: layout(location = 2)
//HULL: out vec3[[TANGENTS:[ ]+[a-zA-Z0-9_]+]][2]
//HULL: layout(location = 4)
//HULL: out vec3[[TANGENT_MATRIX:[ ]+[a-zA-Z0-9_]+]][3][3]
//HULL: layout(location = 13)
//HULL: out vec3[[BITANGENT:[ ]+[a-zA-Z0-9_]+]]

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN: layout(location = 2)
//DOMAIN: in vec3[[DOMAIN_TANGENTS:[ ]+[a-zA-Z0-9_]+]][2]
//DOMAIN: layout(location = 4)
//DOMAIN: in vec3[[DOMAIN_TANGENT_MATRIX:[ ]+[a-zA-Z0-9_]+]][3][3]
//DOMAIN: layout(location = 13)
//DOMAIN: in vec3[[DOMAIN_BITANGENT:[ ]+[a-zA-Z0-9_]+]]

struct VsOut
{
    float3 pos;
}

struct HsOut
{
    float3 pos;
    float2 uv;
}

struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
    float3 tangents[2] : TANGENT;
    float3 tangent_matrix[3][3] : TANGENT_MATRIX;
    float3 bitangent : BITANGENT;
}

struct DsOut
{
    float4 pos : SV_Position;
}

HscOut constants(InputPatch<VsOut, 3> ip)
{
    HscOut o;
    o.edges[0] = 1.0;
    o.edges[1] = 2.0;
    o.edges[2] = 3.0;
    o.inner = 4.0;
    o.tangents[0] = float3(1, 0, 0);
    o.tangents[1] = float3(0, 1, 0);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            o.tangent_matrix[i][j] = float3(1, 0, 0);
    o.bitangent = float3(0, 0, 1);
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
[shader("hull")]
HsOut hullMain(InputPatch<VsOut, 3> ip, uint cpid : SV_OutputControlPointID)
{
    HsOut o;
    o.pos = ip[cpid].pos;
    o.uv = float2(0.0, 1.0);
    return o;
}

[domain("tri")]
DsOut domainMain(
    float3 bary : SV_DomainLocation,
    OutputPatch<HsOut, 3> patch,
    const HscOut patchConstants
)
{
    DsOut o;
    float3 p0 = patch[0].pos;
    float3 p1 = patch[1].pos;
    float3 p2 = patch[2].pos;
    float3 normal = patchConstants.tangents[0] * bary.x + patchConstants.tangents[1] * bary.y
        + patchConstants.tangent_matrix[0][0] + patchConstants.tangent_matrix[2][2];
    o.pos = float4(p0 + p1 + p2 + normal + patchConstants.bitangent, 1.0);
    return o;
}
