// Minimal test case for tessellation shader BuiltinCast issue

struct ConstantsHSOutput
{
    float TessLevelOuter[3] : SV_TessFactor;
    float TessLevelInner : SV_InsideTessFactor;  // This should cause the issue
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV;
};

struct HSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV;
};

ConstantsHSOutput ConstantsHS()
{
    ConstantsHSOutput output;
    output.TessLevelInner = 4.0;
    output.TessLevelOuter[0] = 4.0;
    output.TessLevelOuter[1] = 4.0;
    output.TessLevelOuter[2] = 4.0;
    return output;
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantsHS")]
HSOutput hullMain(InputPatch<VSOutput, 3> patch, uint InvocationID: SV_OutputControlPointID)
{
    HSOutput output;
    output.Pos = patch[InvocationID].Pos;
    output.UV = patch[InvocationID].UV;
    return output;
}