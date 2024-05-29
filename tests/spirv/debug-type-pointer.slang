//TEST(compute, vulkan):SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target spirv -emit-spirv-directly -g2

// This test is to check if DebugTypePointer is emitted when "-g2" option is used

// It looks like the pointer feature is not working properly
// We need to fix and test later. issue#4243
//#define TEST_when_pointer_works

//TEST_INPUT:set inputBuffer = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Node
{
    //SPV: [[STRING_float:%[1-9][0-9]*]] = OpString "float"
    //SPV: [[STRING_pValue:%[1-9][0-9]*]] = OpString "pValue"
    //SPV: [[STRING_uintptr:%[1-9][0-9]*]] = OpString "uintptr"
    //SPV: [[STRING_pNext:%[1-9][0-9]*]] = OpString "pNext"

    float value;

    //SPV: [[TYPE_float:%[1-9][0-9]*]] = OpExtInst %void %{{[0-9]*}} DebugTypeBasic [[STRING_float]]
    //SPV: [[TYPE_float_ptr:%[1-9][0-9]*]] = OpExtInst %void %{{[0-9]*}} DebugTypePointer [[TYPE_float]]
    //SPV: DebugTypeMember [[STRING_pValue]] [[TYPE_float_ptr]]
    float* pValue;

    //SPV: [[TYPE_uintptr:%[1-9][0-9]*]] = OpExtInst %void %{{[0-9]*}} DebugTypeBasic [[STRING_uintptr]]
    //SPV: DebugTypeMember [[STRING_pNext]] [[TYPE_uintptr]]
    Node* pNext;
};

[numthreads(1,1,1)]
void computeMain()
{
    //BUF: 2
    Node node;

    node.value = inputBuffer.Load<float>(4); // 2.0

#if defined(TEST_when_pointer_works)
    node.pValue = &(node.value);
    outputBuffer[0] = *(node.pValue) * node.value; // 4.0
#else
    outputBuffer[0] = node.value; // 2.0
#endif
}

