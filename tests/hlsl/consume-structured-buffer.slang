
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_450 -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -profile glsl_450 -stage compute -entry computeMain

//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type -xslang -fvk-use-gl-layout

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

ConsumeStructuredBuffer<float2> consumeBuffer;

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

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var v = consumeBuffer.Consume();
    outputBuffer[dispatchThreadID.x] = v.x; // expect 1.0
}
