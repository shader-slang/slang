//TEST:SIMPLE(filecheck=HULL):-target spirv -entry hullMain -stage hull
//TEST:SIMPLE(filecheck=DOMAIN):-target spirv -entry domainMain -stage domain

// A `void` hull can write its control-point outputs through global `out` variables. Those globals become
// the fields of the hull's result, and the result layout records the Locations they occupy, so the
// patch-constant outputs start after `gNormal` at Location 1. The domain declares
// `OutputPatch<ControlPoint, N>` first and gives the same fields the same Locations.

//HULL-DAG: OpDecorate %{{[A-Za-z_]*}}gNormal Location 0
//HULL-DAG: OpDecorate %{{[A-Za-z_]*}}color Location 1
//HULL-DAG: OpDecorate %{{[A-Za-z_]*}}color Patch
//HULL-DAG: OpDecorate %{{[A-Za-z_]*}}extra Location 2
//HULL-DAG: OpDecorate %{{[A-Za-z_]*}}extra Patch

//DOMAIN-DAG: OpDecorate %{{[A-Za-z_]*}}normal Location 0
//DOMAIN-DAG: OpDecorate %{{[A-Za-z_]*}}color Location 1
//DOMAIN-DAG: OpDecorate %{{[A-Za-z_]*}}color Patch
//DOMAIN-DAG: OpDecorate %{{[A-Za-z_]*}}extra Location 2
//DOMAIN-DAG: OpDecorate %{{[A-Za-z_]*}}extra Patch

struct ControlPoint
{
    float4 pos : SV_Position;
    float3 normal : NORMAL;
};

struct PatchData
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
    float color : COLOR;
    float4 extra : EXTRA;
};

PatchData constants()
{
    PatchData p;
    p.edges = { 1, 1, 1 };
    p.inside = 1;
    p.color = 0.5;
    p.extra = float4(1, 2, 3, 4);
    return p;
}

out float3 gNormal;

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
void hullMain(InputPatch<ControlPoint, 3> input, uint id : SV_OutputControlPointID)
{
    gNormal = input[id].normal;
}

[shader("domain")]
[domain("tri")]
float4 domainMain(OutputPatch<ControlPoint, 3> patch, PatchData data, float3 bary : SV_DomainLocation)
    : SV_Position
{
    return patch[0].pos * bary.x + float4(patch[1].normal, 0) + data.extra * data.color;
}
