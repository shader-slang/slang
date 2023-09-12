//DISABLE_TEST:SIMPLE(filecheck=GLSL):-line-directive-mode none -target glsl -profile glsl_450 -stage compute -entry computeMain -fvk-u-shift 10 0
//DISABLE_TEST:SIMPLE(filecheck=SPIRV):-line-directive-mode none -target spirv -profile glsl_450 -stage compute -entry computeMain
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -use-dxil -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -fvk-use-gl-layout

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

AppendStructuredBuffer<float2> appendBuffer;

// GLSL:      layout(std430, binding = 11) buffer StructuredBuffer_float2_t
// GLSL-NEXT:     vec2 _data[];
// GLSL-NEXT: } appendBuffer_elements_0

// GLSL:      layout(std430, binding = 12) buffer StructuredBuffer_int_t
// GLSL-NEXT:     int _data[];
// GLSL-NEXT: } appendBuffer_counter

// GLSL:      void AppendStructuredBuffer_Append_0(vec2 [[PARAM:[A-Za-z0-9_]+]])
// GLSL-NEXT: {
// GLSL-NEXT:     int [[COUNTER:[A-Za-z0-9_]+]] = atomicAdd(appendBuffer_counter_0._data[0], 1);
// GLSL-NEXT:     appendBuffer_elements_0._data{{\[}}[[COUNTER]]{{\]}} = [[PARAM]];

// GLSL:      uvec2 StructuredBuffer_GetDimensions_0()
// GLSL-NEXT: {
// GLSL-NEXT:     return uvec2(uint(appendBuffer_counter_0._data[0]), 8U);
// GLSL-NEXT: }

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
    // BUF: type: float
    // BUF-NEXT: 1.0
}
