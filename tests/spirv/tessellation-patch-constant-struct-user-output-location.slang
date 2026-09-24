//TEST:SIMPLE(filecheck=HULL):  -target spirv -entry hullMain  -stage hull   -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target spirv -entry domainMain -stage domain -allow-glsl

// Hull shader: the struct-typed field `n` occupies 2 locations (n.a and
// n.b), so the field after it (tail) must start past that span; reserving
// only one slot for `n` would put tail at location 3, on top of n.b.
//HULL: OpDecorate %entryPointParam_constants_n_a Location 2
//HULL: OpDecorate %entryPointParam_constants_n_a Patch
//HULL: OpDecorate %entryPointParam_constants_n_b Location 3
//HULL: OpDecorate %entryPointParam_constants_n_b Patch
//HULL: OpDecorate %entryPointParam_constants_tail Location 4
//HULL: OpDecorate %entryPointParam_constants_tail Patch
//HULL-NOT: OpDecorate %entryPointParam_constants_tail Location 3

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN: OpDecorate %patchConstants_n_a Location 2
//DOMAIN: OpDecorate %patchConstants_n_a Patch
//DOMAIN: OpDecorate %patchConstants_n_b Location 3
//DOMAIN: OpDecorate %patchConstants_n_b Patch
//DOMAIN: OpDecorate %patchConstants_tail Location 4
//DOMAIN: OpDecorate %patchConstants_tail Patch
//DOMAIN-NOT: OpDecorate %patchConstants_tail Location 3

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