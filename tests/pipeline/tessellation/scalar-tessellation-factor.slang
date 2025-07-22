//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage hull -entry hullMain -warnings-disable 39001

// Test for issue #7000: scalar tessellation factors should convert to arrays properly

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Normal;
    float2 UV;
};

struct HSOutput
{
    float4 Pos : SV_POSITION;
    float3 Normal;
    float2 UV;
};

struct ConstantsHSOutput
{
    float TessLevelOuter[3] : SV_TessFactor;
    float TessLevelInner : SV_InsideTessFactor;  // Scalar (should convert to float[2] for GLSL)
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
    output.Normal = patch[InvocationID].Normal;
    output.UV = patch[InvocationID].UV;
    return output;
}

// CHECK: OpExecutionMode
// CHECK-DAG: OpDecorate {{.*}} BuiltIn TessLevelInner
// CHECK-DAG: OpDecorate {{.*}} BuiltIn TessLevelOuter