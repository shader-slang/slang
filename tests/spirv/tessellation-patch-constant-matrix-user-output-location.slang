//TEST:SIMPLE(filecheck=HULL):  -target spirv -entry hullMain  -stage hull   -allow-glsl
//TEST:SIMPLE(filecheck=DOMAIN):-target spirv -entry domainMain -stage domain -allow-glsl
//TEST:SIMPLE(filecheck=HULL_ROW):  -target spirv -entry hullMain  -stage hull   -allow-glsl -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DOMAIN_ROW):-target spirv -entry domainMain -stage domain -allow-glsl -matrix-layout-row-major

// Hull shader: a matrix patch user output must reserve the same span of
// locations the matching domain input is given (see getSimpleVaryingParameterTypeLayout), or
// the two stages disagree. Varying matrices are sized along the storage-major
// axis: column count under column-major layout (the slangc/test default), row
// count under row-major layout. Without this, each matrix reserved one slot and
// the following user semantics landed inside the matrix's span.
//
// Default (column-major) hull locations:
//HULL: OpDecorate %entryPointParam_constants_basis Location 2
//HULL: OpDecorate %entryPointParam_constants_basis Patch
//HULL: OpDecorate %entryPointParam_constants_shear Location 5
//HULL: OpDecorate %entryPointParam_constants_shear Patch
//HULL: OpDecorate %entryPointParam_constants_tangent Location 9
//HULL: OpDecorate %entryPointParam_constants_tangent Patch
//HULL-NOT: OpDecorate %entryPointParam_constants_shear Location 3
//HULL-NOT: OpDecorate %entryPointParam_constants_tangent Location 4
//HULL-NOT: OpDecorate %entryPointParam_constants_bitangent Location 5
//HULL-NOT: OpDecorate %entryPointParam_constants_bitangent Location 6
//HULL: OpDecorate %entryPointParam_constants_bitangent Location 13
//HULL: OpDecorate %entryPointParam_constants_bitangent Patch
// Row-major hull locations:
//HULL_ROW: OpDecorate %entryPointParam_constants_basis Location 2
//HULL_ROW: OpDecorate %entryPointParam_constants_basis Patch
//HULL_ROW: OpDecorate %entryPointParam_constants_shear Location 5
//HULL_ROW: OpDecorate %entryPointParam_constants_shear Patch
//HULL_ROW: OpDecorate %entryPointParam_constants_tangent Location 7
//HULL_ROW: OpDecorate %entryPointParam_constants_tangent Patch
//HULL_ROW-NOT: OpDecorate %entryPointParam_constants_shear Location 3
//HULL_ROW-NOT: OpDecorate %entryPointParam_constants_tangent Location 4
//HULL_ROW-NOT: OpDecorate %entryPointParam_constants_bitangent Location 5
//HULL_ROW-NOT: OpDecorate %entryPointParam_constants_bitangent Location 6
//HULL_ROW: OpDecorate %entryPointParam_constants_bitangent Location 11
//HULL_ROW: OpDecorate %entryPointParam_constants_bitangent Patch

// Domain shader: the matching patch constant inputs carry the same locations.
//DOMAIN: OpDecorate %patchConstants_basis Location 2
//DOMAIN: OpDecorate %patchConstants_basis Patch
//DOMAIN: OpDecorate %patchConstants_shear Location 5
//DOMAIN: OpDecorate %patchConstants_shear Patch
//DOMAIN: OpDecorate %patchConstants_tangent Location 9
//DOMAIN: OpDecorate %patchConstants_tangent Patch
//DOMAIN-NOT: OpDecorate %patchConstants_shear Location 3
//DOMAIN-NOT: OpDecorate %patchConstants_tangent Location 4
//DOMAIN-NOT: OpDecorate %patchConstants_bitangent Location 5
//DOMAIN-NOT: OpDecorate %patchConstants_bitangent Location 6
//DOMAIN: OpDecorate %patchConstants_bitangent Location 13
//DOMAIN: OpDecorate %patchConstants_bitangent Patch
//DOMAIN_ROW: OpDecorate %patchConstants_basis Location 2
//DOMAIN_ROW: OpDecorate %patchConstants_basis Patch
//DOMAIN_ROW: OpDecorate %patchConstants_shear Location 5
//DOMAIN_ROW: OpDecorate %patchConstants_shear Patch
//DOMAIN_ROW: OpDecorate %patchConstants_tangent Location 7
//DOMAIN_ROW: OpDecorate %patchConstants_tangent Patch
//DOMAIN_ROW-NOT: OpDecorate %patchConstants_shear Location 3
//DOMAIN_ROW-NOT: OpDecorate %patchConstants_tangent Location 4
//DOMAIN_ROW-NOT: OpDecorate %patchConstants_bitangent Location 5
//DOMAIN_ROW-NOT: OpDecorate %patchConstants_bitangent Location 6
//DOMAIN_ROW: OpDecorate %patchConstants_bitangent Location 11
//DOMAIN_ROW: OpDecorate %patchConstants_bitangent Patch

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