//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk

// METAL: (matrix<float,int(4),int(4)>{{.*}}(0) -

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name output
RWStructuredBuffer<float4> output;

[numthreads(1,1,1)]
void computeMain(uniform float4x4 m)
{
    // This test checks for negative zero, so we need to do a hex comparison
    // instead of a float comparison.
    // BUF-COUNT-4: 80000000
    output[0] = (-m)[0];
}
