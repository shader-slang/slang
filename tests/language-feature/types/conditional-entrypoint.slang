//TEST:SIMPLE(filecheck=CHECK_IN_TRUE_SPV): -target spirv -entry mainTrue
//TEST:SIMPLE(filecheck=CHECK_IN_TRUE_HLSL): -target hlsl -entry mainTrue
//TEST:SIMPLE(filecheck=CHECK_IN_FALSE_SPV): -target spirv -entry mainFalse
//TEST:SIMPLE(filecheck=CHECK_IN_FALSE_HLSL): -target hlsl -entry mainFalse
//TEST:SIMPLE(filecheck=CHECK_OUT_TRUE_SPV): -target spirv -entry mainOutputTrue
//TEST:SIMPLE(filecheck=CHECK_OUT_TRUE_HLSL): -target hlsl -entry mainOutputTrue
//TEST:SIMPLE(filecheck=CHECK_OUT_FALSE_SPV): -target spirv -entry mainOutputFalse
//TEST:SIMPLE(filecheck=CHECK_OUT_FALSE_HLSL): -target hlsl -entry mainOutputFalse
//TEST:SIMPLE(filecheck=CHECK_MULTI_SPV): -target spirv -entry mainMulti
//TEST:SIMPLE(filecheck=CHECK_MULTI_HLSL): -target hlsl -entry mainMulti
//TEST:SIMPLE(filecheck=CHECK_LINK_TRUE_SPV): -target spirv -entry mainLinkTimeTrue
//TEST:SIMPLE(filecheck=CHECK_LINK_TRUE_HLSL): -target hlsl -entry mainLinkTimeTrue
//TEST:SIMPLE(filecheck=CHECK_LINK_FALSE_SPV): -target spirv -entry mainLinkTimeFalse
//TEST:SIMPLE(filecheck=CHECK_LINK_FALSE_HLSL): -target hlsl -entry mainLinkTimeFalse

extern static const bool hasInputTrue = true;
extern static const bool hasInputFalse = false;

struct VSInput
{
    float4 Position : POSITION0;
}
struct VSNormal
{
    float4 Normal : NORMAL0;
}
struct VSOutput
{
    float4 Position : SV_Position;
}

// CHECK_IN_TRUE_SPV: OpEntryPoint Vertex
// CHECK_IN_TRUE_SPV: OpVariable {{.*}} Input
// CHECK_IN_TRUE_HLSL: POSITION0
// CHECK_IN_TRUE_HLSL: mainTrue({{.+}})
[shader("vertex")]
VSOutput mainTrue(in Conditional<VSInput, true> optInput)
{
    if (let input = optInput.get())
    {
        VSOutput output;
        output.Position = input.Position;
        return output;
    }
    else
    {
        VSOutput output;
        output.Position = float4(0, 0, 0, 1);
        return output;
    }
}

// CHECK_IN_FALSE_SPV: OpEntryPoint Vertex
// CHECK_IN_FALSE_SPV-NOT: OpVariable {{.*}} Input
// CHECK_IN_FALSE_HLSL-NOT: POSITION
// CHECK_IN_FALSE_HLSL: mainFalse(
[shader("vertex")]
VSOutput mainFalse(in Conditional<VSInput, false> optInput)
{
    if (let input = optInput.get())
    {
        VSOutput output;
        output.Position = input.Position;
        return output;
    }
    else
    {
        VSOutput output;
        output.Position = float4(0, 0, 0, 1);
        return output;
    }
}

// CHECK_OUT_TRUE_SPV: OpEntryPoint Vertex
// CHECK_OUT_TRUE_SPV: OpVariable {{.*}} Output
// CHECK_OUT_TRUE_HLSL: SV_Position
// CHECK_OUT_TRUE_HLSL: mainOutputTrue(
[shader("vertex")]
Conditional<VSOutput, true> mainOutputTrue(VSInput input)
{
    VSOutput output;
    output.Position = input.Position;
    return output;
}

// CHECK_OUT_FALSE_SPV: OpEntryPoint Vertex
// CHECK_OUT_FALSE_SPV-NOT: OpVariable {{.*}} Output
// CHECK_OUT_FALSE_HLSL-NOT: SV_Position
// CHECK_OUT_FALSE_HLSL: void mainOutputFalse(
[shader("vertex")]
Conditional<VSOutput, false> mainOutputFalse(VSInput input)
{
    VSOutput output;
    output.Position = input.Position;
    return output;
}

// CHECK_MULTI_SPV: OpEntryPoint Vertex
// CHECK_MULTI_HLSL: mainMulti(
// CHECK_MULTI_HLSL-NOT: COLOR0
// CHECK_MULTI_HLSL: COLOR1
[shader("vertex")]
VSOutput mainMulti(
    in Conditional<VSInput, true> optInput,
    in Conditional<VSNormal, false> optNormal,
    in Conditional<float3, false> optColor0 : COLOR0,
    in Conditional<float3, true> optColor1 : COLOR1)
{
    VSOutput output;
    if (let input = optInput.get())
        output.Position = input.Position;
    else
        output.Position = float4(0, 0, 0, 1);
    if (let n = optNormal.get())
        output.Position += n.Normal;
    if (let c = optColor0.get())
        output.Position += float4(c, 0);
    if (let c = optColor1.get())
        output.Position += float4(c, 0);
    return output;
}

// CHECK_LINK_TRUE_SPV: OpEntryPoint Vertex
// CHECK_LINK_TRUE_SPV: OpVariable {{.*}} Input
// CHECK_LINK_TRUE_HLSL: POSITION0
// CHECK_LINK_TRUE_HLSL: mainLinkTimeTrue({{.+}})
[shader("vertex")]
VSOutput mainLinkTimeTrue(in Conditional<VSInput, hasInputTrue> optInput)
{
    if (let input = optInput.get())
    {
        VSOutput output;
        output.Position = input.Position;
        return output;
    }
    else
    {
        VSOutput output;
        output.Position = float4(0, 0, 0, 1);
        return output;
    }
}

// CHECK_LINK_FALSE_SPV: OpEntryPoint Vertex
// CHECK_LINK_FALSE_SPV-NOT: OpVariable {{.*}} Input
// CHECK_LINK_FALSE_HLSL-NOT: POSITION
// CHECK_LINK_FALSE_HLSL: mainLinkTimeFalse(
[shader("vertex")]
VSOutput mainLinkTimeFalse(in Conditional<VSInput, hasInputFalse> optInput)
{
    if (let input = optInput.get())
    {
        VSOutput output;
        output.Position = input.Position;
        return output;
    }
    else
    {
        VSOutput output;
        output.Position = float4(0, 0, 0, 1);
        return output;
    }
}
