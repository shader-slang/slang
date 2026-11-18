//TEST:SIMPLE(filecheck=HULL):-target glsl -entry main -stage hull
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain

// Patch constant func output with user-semantics, should NOT be Location 0,
// and each user semantic should get its own distinct location:
//HULL: patch
//HULL: layout(location = 2)
//HULL: out vec3[[HULL_CENTER:[ ]+[a-zA-Z0-9_]+]];
//HULL: patch
//HULL: layout(location = 3)
//HULL: out vec3[[HULL_TANGENT:[ ]+[a-zA-Z0-9_]+]];

// Per-vertex outputs:
//HULL-NOT: {{^patch$}}
//HULL: layout(location = 0)
//HULL: out vec3[[HULL_POS:[ ]+[a-zA-Z0-9_]+]]
//HULL-NOT: {{^patch$}}
//HULL: layout(location = 1)
//HULL: out vec2[[HULL_UV:[ ]+[a-zA-Z0-9_]+]]

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN-NOT: {{^patch$}}
//DOMAIN: layout(location = 0)
//DOMAIN: in vec3[[DOMAIN_POS:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: patch
//DOMAIN: layout(location = 2)
//DOMAIN: in vec3[[DOMAIN_CENTER:[ ]+[a-zA-Z0-9_]+]];
//DOMAIN: patch
//DOMAIN: layout(location = 3)
//DOMAIN: in vec3[[DOMAIN_TANGENT:[ ]+[a-zA-Z0-9_]+]];

struct VOut { float3 pos : POSITION; }
struct HsOut
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
}
struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
    float3 center : CENTER;
    float3 tangent : TANGENT;
}

HscOut constants(InputPatch<VOut, 3> ip, OutputPatch<HsOut, 3> op)
{
    HscOut o;
    o.edges[0] = op[0].pos.x;
    o.edges[1] = 2;
    o.edges[2] = 3;
    o.inner = ip[0].pos.y;
    o.center = float3(42, 99, 7);
    o.tangent = float3(1, 0, 0);
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
[shader("hull")]
HsOut main(InputPatch<VOut, 3> ip, uint cpid : SV_OutputControlPointID)
{
    HsOut o;
    o.pos = ip[cpid].pos;
    o.uv = float2(0.0, 1.0);
    return o;
}

struct DsOut
{
    float4 pos : SV_Position;
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
    interpolated += patchConstants.tangent;
    o.pos = float4(interpolated, 1.0);
    return o;
}