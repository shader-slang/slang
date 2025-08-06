//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -capability vk_mem_model

// Tests if we pass-through and handle buffers correctly.
// Ensure SPIRV emits coherent operations here
// SPIRV: MakePointerVisible
// SPIRV: MakePointerAvailable

// CHECK: 2

//TEST_INPUT:ubuffer(data=[1 2 3], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain()
{
    Ptr<int> ptrIn = &inputBuffer[0];
    Ptr<int> secondPtrIn = ptrIn;

    Ptr<int> ptrOut = &outputBuffer[0];

    coherentStore(ptrOut, coherentLoad(&secondPtrIn[1], 4, MemoryScope::Device), 4, MemoryScope::Device);
}