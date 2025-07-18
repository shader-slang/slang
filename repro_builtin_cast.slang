// Exact reproduction of the user's issue

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Normal;
    float2 UV;
};

struct HSOutput
{
    [[vk::location(2)]] float4 Pos : SV_POSITION;
    [[vk::location(0)]] float3 Normal;
    [[vk::location(1)]] float2 UV;
};

struct ConstantsHSOutput
{
    float TessLevelOuter[3] : SV_TessFactor;
    float TessLevelInner : SV_InsideTessFactor;  // This should cause the BuiltinCast issue
};

ConstantsHSOutput ConstantsHS()
{
    ConstantsHSOutput output;
    output.TessLevelInner = 1.0;
    output.TessLevelOuter[0] = 1.0;
    output.TessLevelOuter[1] = 1.0;
    output.TessLevelOuter[2] = 1.0;
    return output;
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[maxtessfactor(20.0f)]
[patchconstantfunc("ConstantsHS")]
HSOutput hullMain(InputPatch<VSOutput, 3> patch, uint InvocationID: SV_OutputControlPointID)
{
    HSOutput output;
    output.Pos = patch[InvocationID].Pos;
    output.Normal = patch[InvocationID].Normal;
    output.UV = patch[InvocationID].UV;
    return output;
}