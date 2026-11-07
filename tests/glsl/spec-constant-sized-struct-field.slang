// shader-slang/slang#10214: a struct field sized by a specialization constant on GLSL.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// GLSL: float {{ *}}values_0[SPEC_SIZE_0];

[SpecializationConstant]
const int SPEC_SIZE = 4;

[SpecializationConstant]
const float SPEC_SCALE = 2.0;

struct Data
{
    float values[SPEC_SIZE];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Data d;
    for (int i = 0; i < SPEC_SIZE; i++)
        d.values[i] = float(i) * SPEC_SCALE;
    for (int i = 0; i < SPEC_SIZE; i++)
        outputBuffer[i] = d.values[SPEC_SIZE - 1 - i];
}

// BUF: 6.0
// BUF-NEXT: 4.0
// BUF-NEXT: 2.0
// BUF-NEXT: 0.0
