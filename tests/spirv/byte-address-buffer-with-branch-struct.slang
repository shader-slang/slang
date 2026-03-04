//TEST:COMPARE_COMPUTE(filecheck-buffer=NOBRANCH): -vk -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV_BRANCH): -target spirv-asm -stage compute -entry computeMain -DBRANCH

// VariablePointersStorageBuffer is not working on the current GCP servers; driver 573.07.
// github issue #8876
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=BRANCH): -vk -shaderobj -output-using-type -Xslang -DBRANCH

// This tests if a dynamic branching works for the legalization from ByteAddressBuffer to StructurredBuffer.

//SPIRV_BRANCH: OpCapability VariablePointersStorageBuffer

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0]):out,name outputBuffer
uniform RWStructuredBuffer<float4> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11 12 13], stride=4):name input1
uniform ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[20 21 22 23], stride=4):name input2
uniform ByteAddressBuffer input2;

struct NestedObject
{
    ByteAddressBuffer b;
}

struct SomeObject
{
    NestedObject nested;
}

ByteAddressBuffer struct2buffer(int tid, SomeObject o1, SomeObject o2)
{
#if defined(BRANCH)
    if (tid == 0)
        return o2.nested.b;
#endif

    return o1.nested.b;
}

void testStruct2Buffer(int tid)
{
    SomeObject o1, o2;
    o1.nested.b = input1;
    o2.nested.b = input2;
    ByteAddressBuffer b = struct2buffer(tid, o1, o2);
    let data = b.Load<uint4>(0);
    outputBuffer[0] = data;
    // NOBRANCH: 10.0
    // NOBRANCH: 11.0
    // NOBRANCH: 12.0
    // NOBRANCH: 13.0
    // BRANCH: 20.0
    // BRANCH: 21.0
    // BRANCH: 22.0
    // BRANCH: 23.0
}

SomeObject struct2struct(int tid, SomeObject o1, SomeObject o2)
{
#if defined(BRANCH)
    if (tid == 0)
        return o2;
#endif

    return o1;
}

void testStruct2Struct(int tid)
{
    SomeObject o1, o2;
    o1.nested.b = input1;
    o2.nested.b = input2;
    SomeObject b = struct2struct(tid, o1, o2);
    let data = b.nested.b.Load<uint4>(0);
    outputBuffer[1] = data;
    // NOBRANCH: 10.0
    // NOBRANCH: 11.0
    // NOBRANCH: 12.0
    // NOBRANCH: 13.0
    // BRANCH: 20.0
    // BRANCH: 21.0
    // BRANCH: 22.0
    // BRANCH: 23.0
}

void testBuffer2Struct(int tid)
{
    SomeObject o;
    o.nested.b = input1;

#if defined(BRANCH)
    if (tid == 0)
        o.nested.b = input2;
#endif

    let data = o.nested.b.Load<uint4>(0);
    outputBuffer[2] = data;
    // NOBRANCH: 10.0
    // NOBRANCH: 11.0
    // NOBRANCH: 12.0
    // NOBRANCH: 13.0
    // BRANCH: 20.0
    // BRANCH: 21.0
    // BRANCH: 22.0
    // BRANCH: 23.0
}


[numthreads(1,1,1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    testStruct2Buffer(tid.x);
    testStruct2Struct(tid.x);
    testBuffer2Struct(tid.x);
}
