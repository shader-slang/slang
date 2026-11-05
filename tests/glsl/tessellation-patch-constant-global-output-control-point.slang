//TEST:SIMPLE(filecheck=HULL):-target glsl -entry hullMain -stage hull
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain

// A `void` hull can write its control-point outputs through global `out` variables. Those globals
// become the fields of the hull's result, and the result layout records the locations they occupy,
// so the `patch` outputs start after `gNormal` at location 1. The domain declares
// `OutputPatch<ControlPoint, N>` first and gives the same fields the same locations.

//HULL: {{^}}patch{{$}}
//HULL-NEXT: layout(location = 1)
//HULL-NEXT: out float {{.*}}color
//HULL: {{^}}patch{{$}}
//HULL-NEXT: layout(location = 2)
//HULL-NEXT: out vec4 {{.*}}extra

//DOMAIN: {{^}}patch{{$}}
//DOMAIN-NEXT: layout(location = 1)
//DOMAIN-NEXT: in float {{.*}}color
//DOMAIN: {{^}}patch{{$}}
//DOMAIN-NEXT: layout(location = 2)
//DOMAIN-NEXT: in vec4 {{.*}}extra

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
