//DISABLE_TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// SPIRV: OpEntryPoint
// SPIRV-NOT: error

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
uniform int4* output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1
    int* v = __getAddress(output[0].x);
    InterlockedAdd(*v, 1);
}