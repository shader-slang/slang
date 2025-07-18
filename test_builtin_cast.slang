// Test case to reproduce the BuiltinCast issue

struct ConstantsHSOutput
{
    float TessLevelOuter[3] : SV_TessFactor;
    float TessLevelInner : SV_InsideTessFactor;  // This should cause the issue
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
[patchconstantfunc("ConstantsHS")]
void hullMain()
{
}