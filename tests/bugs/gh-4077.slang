//TEST(compute, vulkan):SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target spirv -emit-spirv-directly -g2

// This test is to check if DebugTypePointer is emitted when "-g2" option is
// used and the shader uses a memory pointer.

// It looks like the pointer feature is not working properly
// We need to fix and test later.
//#define TEST_when_pointer_works_better

//TEST_INPUT:set inputBuffer = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Node
{
    float value;

    //SPV: OpExtInst %void %{{[0-9]*}} DebugTypePointer
    float* pValue;
};

[numthreads(1,1,1)]
void computeMain()
{
    //BUF: 2
    Node node;

    node.value = inputBuffer.Load<float>(4); // 2.0

#if defined(TEST_when_pointer_works_better)
    node.pValue = &(node.value);
    outputBuffer[0] = *(node.pValue) * node.value; // 4.0
#else
    outputBuffer[0] = node.value; // 2.0
#endif
}

