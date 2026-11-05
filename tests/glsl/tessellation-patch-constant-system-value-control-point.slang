//TEST:SIMPLE(filecheck=HULL):-target glsl -entry hullMain -stage hull
//TEST:SIMPLE(filecheck=HULL):-target glsl -entry hullStruct -stage hull
//TEST:SIMPLE(filecheck=DOMAIN):-target glsl -entry domainMain -stage domain

// A control-point output made only of system values (gl_Position, gl_ClipDistance) occupies no
// location, so the hull numbers its `patch` outputs from 0. The domain stage gives the same
// patch-constant fields the same locations, which the two stages must agree on.
// `domainMain` declares `PatchData` before `OutputPatch` on purpose: with no location in the
// control point, the order does not change where the patch-constant fields start.

//HULL: {{^}}patch{{$}}
//HULL-NEXT: layout(location = 0)
//HULL-NEXT: out float {{.*}}color
//HULL: {{^}}patch{{$}}
//HULL-NEXT: layout(location = 1)
//HULL-NEXT: out vec4 {{.*}}extra

//DOMAIN: {{^}}patch{{$}}
//DOMAIN-NEXT: layout(location = 0)
//DOMAIN-NEXT: in float {{.*}}color
//DOMAIN: {{^}}patch{{$}}
//DOMAIN-NEXT: layout(location = 1)
//DOMAIN-NEXT: in vec4 {{.*}}extra

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
