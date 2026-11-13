//TEST:SIMPLE(filecheck=WIDE):-target spirv -entry hullWide -stage hull
//TEST:SIMPLE(filecheck=WIDE):-target spirv -entry domainWide -stage domain
//TEST:SIMPLE(filecheck=EXPLICIT):-target spirv -entry hullExplicit -stage hull
//TEST:SIMPLE(filecheck=EXPLICIT):-target spirv -entry domainExplicit -stage domain

// The hull numbers its patch-constant outputs after the Locations that the front-end layout gives
// its output control point, and the domain lays out `OutputPatch<ControlPoint, N>` with the same
// rules. A `float4x4` takes 4 Locations, and explicit Locations 0 and 2 take 3 (the gap at 1 is
// not reused), so both stages agree on where the patch-constant fields start. Both domains
// declare `OutputPatch` before `PatchData`; a domain that declares `PatchData` first numbers the
// patch-constant fields from 0 and does not match these hulls.

//WIDE-DAG: OpDecorate %{{[A-Za-z_]*}}color Location 4
//WIDE-DAG: OpDecorate %{{[A-Za-z_]*}}color Patch
//WIDE-DAG: OpDecorate %{{[A-Za-z_]*}}extra Location 5
//WIDE-DAG: OpDecorate %{{[A-Za-z_]*}}extra Patch

//EXPLICIT-DAG: OpDecorate %{{[A-Za-z_]*}}color Location 3
//EXPLICIT-DAG: OpDecorate %{{[A-Za-z_]*}}color Patch
//EXPLICIT-DAG: OpDecorate %{{[A-Za-z_]*}}extra Location 4
//EXPLICIT-DAG: OpDecorate %{{[A-Za-z_]*}}extra Patch

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
