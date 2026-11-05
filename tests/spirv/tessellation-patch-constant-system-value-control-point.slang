//TEST:SIMPLE(filecheck=HULL):-target spirv -entry hullMain -stage hull
//TEST:SIMPLE(filecheck=HULL):-target spirv -entry hullStruct -stage hull
//TEST:SIMPLE(filecheck=DOMAIN):-target spirv -entry domainMain -stage domain

// A control-point output made only of system values (gl_Position, gl_ClipDistance) occupies no
// Location, so the hull numbers its patch-constant outputs from 0. The domain stage gives the
// same patch-constant fields the same Locations, which the two stages must agree on.

//HULL-DAG: OpDecorate %entryPointParam_constants_color Location 0
//HULL-DAG: OpDecorate %entryPointParam_constants_color Patch
//HULL-DAG: OpDecorate %entryPointParam_constants_extra Location 1
//HULL-DAG: OpDecorate %entryPointParam_constants_extra Patch

//DOMAIN-DAG: OpDecorate %data_color Location 0
//DOMAIN-DAG: OpDecorate %data_color Patch
//DOMAIN-DAG: OpDecorate %data_extra Location 1
//DOMAIN-DAG: OpDecorate %data_extra Patch

struct ControlPoint
{
    float4 pos : SV_Position;
    float clip[1] : SV_ClipDistance;
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

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
float4 hullMain(InputPatch<float4, 3> input, uint id : SV_OutputControlPointID) : SV_Position
{
    return input[id];
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
ControlPoint hullStruct(InputPatch<ControlPoint, 3> input, uint id : SV_OutputControlPointID)
{
    return input[id];
}

[shader("domain")]
[domain("tri")]
float4 domainMain(PatchData data, OutputPatch<ControlPoint, 3> patch, float3 bary : SV_DomainLocation)
    : SV_Position
{
    return patch[0].pos * bary.x + data.extra * data.color;
}
