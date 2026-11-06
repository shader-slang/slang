//TEST:SIMPLE(filecheck=WIDE):-target glsl -entry hullWide -stage hull
//TEST:SIMPLE(filecheck=WIDE):-target glsl -entry domainWide -stage domain
//TEST:SIMPLE(filecheck=EXPLICIT):-target glsl -entry hullExplicit -stage hull
//TEST:SIMPLE(filecheck=EXPLICIT):-target glsl -entry domainExplicit -stage domain

// The hull numbers its `patch` outputs after the locations that the front-end layout gives its
// output control point, and the domain lays out `OutputPatch<ControlPoint, N>` with the same
// rules. A `float4x4` takes 4 locations, and explicit locations 0 and 2 take 3 (the gap at 1 is
// not reused), so both stages agree on where the patch-constant fields start. Both domains
// declare `OutputPatch` before `PatchData`; a domain that declares `PatchData` first numbers the
// patch-constant fields from 0 and does not match these hulls.

//WIDE: {{^}}patch{{$}}
//WIDE-NEXT: layout(location = 4)
//WIDE-NEXT: {{in|out}} float {{.*}}color
//WIDE: {{^}}patch{{$}}
//WIDE-NEXT: layout(location = 5)
//WIDE-NEXT: {{in|out}} vec4 {{.*}}extra

//EXPLICIT: {{^}}patch{{$}}
//EXPLICIT-NEXT: layout(location = 3)
//EXPLICIT-NEXT: {{in|out}} float {{.*}}color
//EXPLICIT: {{^}}patch{{$}}
//EXPLICIT-NEXT: layout(location = 4)
//EXPLICIT-NEXT: {{in|out}} vec4 {{.*}}extra

struct WideControlPoint
{
    float4 pos : SV_Position;
    float4x4 m : MATRIX;
};

struct ExplicitControlPoint
{
    float4 pos : SV_Position;
    [[vk::location(0)]] float4 a : A;
    [[vk::location(2)]] float4 b : B;
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
WideControlPoint hullWide(InputPatch<WideControlPoint, 3> input, uint id : SV_OutputControlPointID)
{
    return input[id];
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
ExplicitControlPoint hullExplicit(
    InputPatch<ExplicitControlPoint, 3> input,
    uint id : SV_OutputControlPointID)
{
    return input[id];
}

[shader("domain")]
[domain("tri")]
float4 domainWide(OutputPatch<WideControlPoint, 3> patch, PatchData data, float3 bary : SV_DomainLocation)
    : SV_Position
{
    return patch[0].m[0] * bary.x + data.extra * data.color;
}

[shader("domain")]
[domain("tri")]
float4 domainExplicit(
    OutputPatch<ExplicitControlPoint, 3> patch,
    PatchData data,
    float3 bary : SV_DomainLocation)
    : SV_Position
{
    return patch[0].a * bary.x + patch[1].b + data.extra * data.color;
}
