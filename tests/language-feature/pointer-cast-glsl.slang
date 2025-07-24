//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -emit-spirv-via-glsl

// Test that pointer casts in GLSL generate constructor-style casts instead of C-style casts
// This addresses issue https://github.com/shader-slang/slang/issues/7838

//TEST_INPUT: set address = ubuffer(data=[1 2 3 4 5 6 7 8], stride=4);
uniform uint64_t address;
//TEST_INPUT: set pointer = ubuffer(data=[1 2 3 4 5 6 7 8], stride=4);
uniform int* pointer;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

// Ensure we have a proper GLSL supported ctor call for BDA
//GLSL: BufferPointer__{{[a-zA-Z0-9_]+}}({{[a-zA-Z0-9_]+}}_0)

// Ensure our GLSL code compiles and runs
//BUF: 2

[shader("compute")]
[numthreads(1,1,1)]
void computeMain() : SV_Position
{
    // This should generate BufferPointer(address) instead of (BufferPointer)address in GLSL
    let buffer1 = ConstBufferPointer<int>(address);
    let buffer2 = ConstBufferPointer<int>(pointer);
    outputBuffer[0] = buffer1[0] + buffer2[0];
}
