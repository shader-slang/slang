// Runtime check that stores and atomics through a `T*` reach the pointee when the shader goes
// through GLSL (shader-slang/slang#13393). The direct-SPIR-V leg is the reference.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -output-using-type

struct Pair
{
    int a;
    int b;
}

//TEST_INPUT: set data = ubuffer(data=[1 2 3 4], stride=4);
uniform int* data;
//TEST_INPUT: set pair = ubuffer(data=[5 6], stride=4);
uniform Pair* pair;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    *data = 10;
    data[1] = 20;
    int old;
    InterlockedAdd(data[2], 100, old);
    Pair p;
    p.a = 50;
    p.b = 60;
    *pair = p;

    // CHECK: type: int32_t
    // CHECK-NEXT: {{^}}10{{$}}
    outputBuffer[0] = data[0];
    // CHECK-NEXT: {{^}}20{{$}}
    outputBuffer[1] = data[1];
    // CHECK-NEXT: {{^}}103{{$}}
    outputBuffer[2] = data[2];
    // CHECK-NEXT: {{^}}3{{$}}
    outputBuffer[3] = old;
    // CHECK-NEXT: {{^}}50{{$}}
    outputBuffer[4] = pair->a;
    // CHECK-NEXT: {{^}}60{{$}}
    outputBuffer[5] = pair->b;
}
