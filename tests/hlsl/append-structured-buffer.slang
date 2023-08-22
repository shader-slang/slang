
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_450 -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -profile glsl_450 -stage compute -entry computeMain

//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type -xslang -fvk-use-gl-layout

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

AppendStructuredBuffer<float2> appendBuffer;

// GLSL: layout(std430, binding = 1) buffer StructuredBuffer_float2_t
// GLSL: vec2 _data[];
// GLSL: } appendBuffer_elements_0

// GLSL: layout(std430, binding = 2) buffer StructuredBuffer_int_t
// GLSL: int _data[];
// GLSL: } appendBuffer_counter

// GLSL: void AppendStructuredBuffer_Append_0(vec2 [[PARAM:[A-Za-z0-9_]+]])
// GLSL:    int [[COUNTER:[A-Za-z0-9_]+]] = atomicAdd(appendBuffer_counter_0._data[0], 1);
// GLSL:    appendBuffer_elements_0._data{{\[}}[[COUNTER]]{{\]}} = [[PARAM]];

// GLSL: uvec2 StructuredBuffer_GetDimensions_0()
// GLSL: {
// GLSL:     return uvec2(uint(appendBuffer_counter_0._data[0]), 8U);
// GLSL: }

// SPIRV: OpEntryPoint

//TEST_INPUT:set inBuffer = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4)
RWByteAddressBuffer inBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var g = inBuffer.Load<float4>(4);
    appendBuffer.Append(g.xy);

    uint numStructs, stride;
    appendBuffer.GetDimensions(numStructs, stride);
    outputBuffer[dispatchThreadID.x] = numStructs; // expect 1.0
}
