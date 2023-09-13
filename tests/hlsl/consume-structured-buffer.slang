//DISABLE_TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_450 -stage compute -entry computeMain
//DISABLE_TEST:SIMPLE(filecheck=SPIRV):-target spirv -profile glsl_450 -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -use-dxil -compute -output-using-type
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type -xslang -fvk-use-gl-layout

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// To check that our counter-initialization works correctly, set the initial
// counter to 6 instead of 8, so we'll start reading from 6.0 downwards
//TEST_INPUT:set consumeBuffer = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, counter=6)
ConsumeStructuredBuffer<float> consumeBuffer;

// GLSL: layout(std430, binding = 1) buffer StructuredBuffer_float2_t
// GLSL: vec2 _data[];
// GLSL: } consumeBuffer_elements_0

// GLSL: layout(std430, binding = 2) buffer StructuredBuffer_int_t
// GLSL: int _data[];
// GLSL: } consumeBuffer_counter

// GLSL: vec2 ConsumeStructuredBuffer_Consume_0()
// GLSL:    int [[COUNTER:[A-Za-z0-9_]+]] = atomicAdd(consumeBuffer_counter_0._data[0], -1);
// GLSL:    int [[COUNTER1:[A-Za-z0-9_]+]] = [[COUNTER]] - 1;
// GLSL:    if{{\s?}}([[COUNTER1]] >= 0)
// GLSL:    return consumeBuffer_elements_0._data{{\[}}[[COUNTER1]]{{\]}};
// GLSL:    else
// GLSL:    return vec2(0.0);

// SPIRV: OpEntryPoint

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    uint size, stride;
    consumeBuffer.GetDimensions(size, stride);

    if(i == 0)
        outputBuffer[0] = size;

    outputBuffer[i+1] = consumeBuffer.Consume();

    // BUF: type: float
    // The total size of the ConsumeStructuredBuffer (from GetDimensions)
    // BUF: 8.0

    // The values from consumeBuffer in any order
    // BUF-DAG: 6.0
    // BUF-DAG: 4.0
    // BUF-DAG: 5.0
    // BUF-DAG: 3.0
}
