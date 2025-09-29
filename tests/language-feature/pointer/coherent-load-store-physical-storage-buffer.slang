//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model

// Ensure SPIRV emits coherent operations here
// SPIRV: MakePointerVisible
// SPIRV: MakePointerAvailable

// CHECK: 2

//TEST_INPUT:ubuffer(data=[1 2 3], stride=4):name=inputBuffer
uniform int* inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
uniform int* outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    Ptr<int> ptrIn = inputBuffer;
    Ptr<int> secondPtrIn = ptrIn;
    Ptr<int> ptrOut = outputBuffer;
    storeCoherent<4, MemoryScope::Device>(ptrOut, loadCoherent<4, MemoryScope::Device>(&secondPtrIn[1]));
}