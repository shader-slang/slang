//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -profile glsl_460
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -xslang -g0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-d3d12 -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type

// Check that when generating spirv directly, we do not load the entire big array
// from the constant buffer into registers.

struct WorkData
{
    float A[2*4];
    float B[1024];
    
    float Foo(uint i) { return A[i] * B[i]; }
};

//TEST_INPUT:set input = new WorkData{[1.0, 2.0, 3.0, 4.0], [10.0, 20.0, 30.0, 40.0]}
ConstantBuffer<WorkData> input;

//TEST_INPUT:set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> resultBuffer;

// CHECK-NOT: OpLoopMerge
// CHECK-NOT: OpCompositeConstruct
// CHECK-COUNT-1: OpStore

[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: 10.0
    // BUF: 40.0
    resultBuffer[tid.x] = input.Foo(tid.x);
}