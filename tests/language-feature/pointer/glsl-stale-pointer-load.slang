// A pointer loaded from memory keeps its value when that memory is overwritten before the
// pointer is used (shader-slang/slang#13403). On GLSL the loaded `buffer_reference` handle must
// be captured in a temporary before the store, rather than re-read at the use.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -output-using-type
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

//TEST_INPUT: set oldTarget = ubuffer(data=[0 0], stride=4);
uniform int* oldTarget;
//TEST_INPUT: set newTarget = ubuffer(data=[0 0], stride=4);
uniform int* newTarget;
//TEST_INPUT: set slots = ubuffer(data=[oldTarget], stride=8);
RWStructuredBuffer<int*> slots;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // GLSL: [[P:[A-Za-z0-9_]+]] = slots_0._data[uint(0)];
    // GLSL: slots_0._data[uint(0)] = {{.*}}newTarget_0;
    // GLSL: [[P]]._data = 3;
    int* p = slots[0];
    slots[0] = newTarget;
    *p = 3;

    // GLSL: [[Q:[A-Za-z0-9_]+]] = slots_0._data[uint(0)];
    // GLSL: slots_0._data[uint(0)] = {{.*}}oldTarget_0;
    // GLSL: = [[Q]]._data;
    int* q = slots[0];
    slots[0] = oldTarget;
    int seen = *q;

    // CHECK: type: int32_t
    // CHECK-NEXT: 3
    outputBuffer[0] = *oldTarget;
    // CHECK-NEXT: 0
    outputBuffer[1] = *newTarget;
    // CHECK-NEXT: 0
    outputBuffer[2] = seen;
}
