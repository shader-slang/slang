//TEST:SIMPLE(filecheck=HULL):-target glsl -entry hullMain -stage hull -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain -allow-glsl

// Hull shader: patch constant output should have the `patch` qualifier and a location
//HULL: patch
//HULL: layout(location = 2)
//HULL: out vec3 [[HULL_CENTER:[a-zA-Z0-9_]+]];

// Domain shader: the patch constant input should have the `patch` qualifier carrying the
// same location as the hull output
//DOMAIN: patch
//DOMAIN: layout(location = 2)
//DOMAIN: in vec3 [[DOMAIN_CENTER:[a-zA-Z0-9_]+]];

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
    float3 center;
}

struct DsOut
{
    float4 pos : SV_Position;
}

HscOut constants(InputPatch<VsOut, 3> ip)
{
    HscOut o;
    o.edges[0] = 1.0;
    o.edges[1] = 1.0;
    o.edges[2] = 1.0;
    o.inner = 1.0;
    o.center = float3(ip[0].pos) + float3(ip[1].pos) + float3(ip[2].pos);
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
    float3 interpolated = p0 * bary.x + p1 * bary.y + p2 * bary.z;
    interpolated += patchConstants.center;
    interpolated += float3(patchConstants.edges[0], patchConstants.edges[1], patchConstants.inner);
    o.pos = float4(interpolated, 1.0);
    return o;
}
