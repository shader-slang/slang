//TEST:SIMPLE(filecheck=GLSL): -target glsl -allow-glsl

// Test for issue #7614: ensure const qualifier is present after layout for specialization constants

// The fix should add 'const ' after layout(constant_id = N) for specialization constants
// GLSL: layout(constant_id = 42)
// GLSL-NEXT: const int mySpecConstant_0

[vk::constant_id(42)]
const int mySpecConstant = 123;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = mySpecConstant;
}