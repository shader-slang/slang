//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV): -vk -shaderobj -output-using-type -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPIRV_BRANCH): -target spirv-asm -stage compute -entry computeMain -DBRANCH

// VariablePointersStorageBuffer is not working on the current GCP servers; driver 573.07.
// github issue #8876
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=BRANCH): -vk -shaderobj -output-using-type -emit-spirv-directly -Xslang -DBRANCH

// This tests if a dynamic branching works for the legalization from ByteAddressBuffer to StructurredBuffer.

//SPIRV_BRANCH: OpCapability RuntimeDescriptorArray

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
uniform RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: set input1 = ubuffer(data=[10 11], stride=4)
uniform ByteAddressBuffer.Handle input1;

//TEST_INPUT: set input2 = ubuffer(data=[20 21], stride=4)
uniform ByteAddressBuffer.Handle input2;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    ByteAddressBuffer.Handle buf = input1;

#if defined(BRANCH)
    if (tid.x == 0)
        buf = input2;
#endif

    let result0 = buf.Load<int>(0);
    let result1 = buf.Load<int>(4);

    outputBuffer[0] = result0;
    outputBuffer[1] = result1;

    // SPIRV: 10
    // SPIRV-NEXT: 11

    // BRANCH: 20
    // BRANCH-NEXT: 21
}
