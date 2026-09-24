//TEST:SIMPLE(filecheck=HULL):  -target glsl -entry hullMain  -stage hull   -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain -allow-glsl

// Hull shader: the struct-typed field `n` occupies 2 locations (n.a and
// n.b), so the field after it (tail) must start past that span; reserving
// only one slot for `n` would put tail at location 3, on top of n.b.
//HULL: layout(location = 2)
//HULL: out vec3[[N_A:[ ]+[a-zA-Z0-9_]+]]
//HULL: layout(location = 3)
//HULL: out vec3[[N_B:[ ]+[a-zA-Z0-9_]+]]
//HULL: layout(location = 4)
//HULL: out vec3[[TAIL:[ ]+[a-zA-Z0-9_]+]]

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN: layout(location = 2)
//DOMAIN: in vec3[[DOMAIN_N_A:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: layout(location = 3)
//DOMAIN: in vec3[[DOMAIN_N_B:[ ]+[a-zA-Z0-9_]+]]
//DOMAIN: layout(location = 4)
//DOMAIN: in vec3[[DOMAIN_TAIL:[ ]+[a-zA-Z0-9_]+]]

struct VsOut
{
    float3 pos;
}

struct HsOut
{
    float3 pos;
    float2 uv;
}

struct Nested
{
    float3 a;
    float3 b;
}

struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
    Nested n;
    float3 tail;
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
    o.n.a = float3(1, 0, 0);
    o.n.b = float3(0, 1, 0);
    o.tail = float3(0, 0, 1);
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
    float3 normal = patchConstants.n.a + patchConstants.n.b;
    o.pos = float4(p0 + p1 + p2 + normal + patchConstants.tail, 1.0);
    return o;
}