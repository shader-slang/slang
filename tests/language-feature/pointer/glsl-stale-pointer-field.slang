// A pointer loaded from a field behind another pointer keeps its value when that field is
// overwritten before the pointer is used (shader-slang/slang#13403). On GLSL the loaded
// `buffer_reference` handle must be captured in a temporary before the store.
//
// Vulkan execution of this shape is covered by `glsl-stale-pointer-load.slang`, which reads
// the pointer from a structured buffer instead: when a pointer is stored into a field behind a
// pointer, the bundled glslang marks the 8-byte store `Aligned 4`, which fails SPIR-V
// validation (shader-slang/slang#13407, fixed upstream in KhronosGroup/glslang#4123).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

struct Node
{
    int* ip;
}

//TEST_INPUT: set oldTarget = ubuffer(data=[0 0], stride=4);
uniform int* oldTarget;
//TEST_INPUT: set newTarget = ubuffer(data=[0 0], stride=4);
uniform int* newTarget;
//TEST_INPUT: set node = ubuffer(data=[oldTarget], stride=4);
uniform Node* node;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // GLSL: [[P:[A-Za-z0-9_]+]] = {{.*}}node_0._data.ip_0;
    // GLSL: node_0._data.ip_0 = {{.*}}newTarget_0;
    // GLSL: [[P]]._data = 3;
    int* p = node->ip;
    node->ip = newTarget;
    *p = 3;

    // GLSL: [[Q:[A-Za-z0-9_]+]] = {{.*}}node_0._data.ip_0;
    // GLSL: node_0._data.ip_0 = {{.*}}oldTarget_0;
    // GLSL: = [[Q]]._data;
    int* q = node->ip;
    node->ip = oldTarget;
    int seen = *q;

    // CHECK: 3
    outputBuffer[0] = *oldTarget;
    // CHECK: 0
    outputBuffer[1] = *newTarget;
    // CHECK: 0
    outputBuffer[2] = seen;
}
