//TEST:SIMPLE(filecheck=HULL):  -target glsl -entry hullMain  -stage hull   -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain -allow-glsl
//TEST:SIMPLE(filecheck=HULL_ROW):  -target glsl -entry hullMain  -stage hull   -allow-glsl -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DOMAIN_ROW):-target glsl -entry domainMain -stage domain -allow-glsl -matrix-layout-row-major

// Hull shader: a matrix patch user output must reserve the same span of
// locations the matching domain input is given (see getSimpleVaryingParameterTypeLayout), or
// the two stages disagree. Varying matrices are sized along the storage-major
// axis: column count under column-major layout (the slangc/test default), row
// count under row-major layout. Without this, each matrix reserved one slot and
// the following user semantics landed inside it.
//
// Default (column-major) hull locations:
//HULL: layout(location = 2)
//HULL: out mat3x3[[BASIS:[ ]+[a-zA-Z0-9_]+]]
//HULL: layout(location = 5)
//HULL: out mat2x4[[SHEAR:[ ]+[a-zA-Z0-9_]+]]
//HULL: layout(location = 9)
//HULL: out mat4x4[[TANGENT:[ ]+[a-zA-Z0-9_]+]]
//HULL: layout(location = 13)
//HULL: out vec3[[BITANGENT:[ ]+[a-zA-Z0-9_]+]]
// Row-major hull locations:
//HULL_ROW: layout(location = 2)
//HULL_ROW: out mat3x3[[ROW_BASIS:[ ]+[a-zA-Z0-9_]+]]
//HULL_ROW: layout(location = 5)
//HULL_ROW: out mat2x4[[ROW_SHEAR:[ ]+[a-zA-Z0-9_]+]]
//HULL_ROW: layout(location = 7)
//HULL_ROW: out mat4x4[[ROW_TANGENT:[ ]+[a-zA-Z0-9_]+]]
//HULL_ROW: layout(location = 11)
//HULL_ROW: out vec3[[ROW_BITANGENT:[ ]+[a-zA-Z0-9_]+]]

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN: layout(location = 2)
//DOMAIN: in mat3x3[[DOMAIN_BASIS:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: layout(location = 5)
//DOMAIN: in mat2x4[[DOMAIN_SHEAR:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: layout(location = 9)
//DOMAIN: in mat4x4[[DOMAIN_TANGENT:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: layout(location = 13)
//DOMAIN: in vec3[[DOMAIN_BITANGENT:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN_ROW: layout(location = 2)
//DOMAIN_ROW: in mat3x3[[ROW_DOMAIN_BASIS:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN_ROW: layout(location = 5)
//DOMAIN_ROW: in mat2x4[[ROW_DOMAIN_SHEAR:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN_ROW: layout(location = 7)
//DOMAIN_ROW: in mat4x4[[ROW_DOMAIN_TANGENT:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN_ROW: layout(location = 11)
//DOMAIN_ROW: in vec3[[ROW_DOMAIN_BITANGENT:[ ]+[a-zA-Z0-9_]+]]

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
    float3x3 basis : BASIS;
    float2x4 shear : SHEAR;
    float4x4 tangent : TANGENT;
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
    o.basis = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    o.shear = float2x4(1, 0, 0, 0, 0, 1, 0, 0);
    o.tangent = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
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
    float3 normal = patchConstants.basis[0] + patchConstants.basis[1] + patchConstants.basis[2];
    float4 combined = patchConstants.shear[0] + patchConstants.shear[1] + patchConstants.tangent[3];
    float3 normal2 = normal + combined.xyz + patchConstants.bitangent;
    o.pos = float4(p0 + p1 + p2 + bary.x * normal2, 1.0);
    return o;
}