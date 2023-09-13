//DISABLE_TEST:SIMPLE(filecheck=GLSL):-line-directive-mode none -target glsl -profile glsl_450 -stage compute -entry computeMain -fvk-u-shift 10 0
//DISABLE_TEST:SIMPLE(filecheck=SPIRV):-line-directive-mode none -target spirv -profile glsl_450 -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -use-dxil -compute -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -fvk-use-gl-layout

// To check that our counter-initialization works correctly, set the initial
// counter to 1 instead of 0
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4, counter=1):out,name=outputBuffer
AppendStructuredBuffer<float> outputBuffer;

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

//TEST_INPUT:set inBuffer = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)
RWStructuredBuffer<float> inBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    float g = inBuffer[i];
    outputBuffer.Append(g);

    GroupMemoryBarrier();

    uint numStructs, stride;
    outputBuffer.GetDimensions(numStructs, stride);
    if(i == 0)
        outputBuffer.Append(float(numStructs));

    // BUF: type: float
    // Never assigned, as we set the initial counter to 1
    // BUF: 0.0

    // The values from inBuffer in any order
    // BUF-DAG: 1.0
    // BUF-DAG: 3.0
    // BUF-DAG: 2.0
    // BUF-DAG: 4.0

    // The total size of the AppendStructuredBuffer (from GetDimensions)
    // BUF: 8.0

    // Never assigned
    // BUF: 0.0
    // BUF: 0.0
}
