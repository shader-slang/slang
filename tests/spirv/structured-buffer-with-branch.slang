//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV): -vk -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV_BRANCH): -target spirv-asm -stage compute -entry computeMain -DBRANCH

// VariablePointersStorageBuffer is not working on the current GCP servers; driver 573.07.
// github issue #8876
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=BRANCH): -vk -shaderobj -output-using-type -Xslang -DBRANCH

// This tests if a dynamic branching works for StructurredBuffer.

//SPIRV_BRANCH: OpCapability VariablePointersStorageBuffer

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
uniform RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11], stride=4):name input1
uniform StructuredBuffer<int> input1;

//TEST_INPUT:ubuffer(data=[20 21], stride=4):name input2
uniform StructuredBuffer<int> input2;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    StructuredBuffer<int> buf = input1;

#if defined(BRANCH)
    if (tid.x == 0)
        buf = input2;
#endif
    // When `BRANCH` is not defined, it is easy to inline the global-param, `input1`, and remove `buf`.
    //
    // When `BRANCH` is defined, we cannot inline the global-param, because it is unknown which one
    // should be inlined until the runtime.

    outputBuffer[0] = buf[0];
    outputBuffer[1] = buf[1];

    // SPIRV: 10
    // SPIRV-NEXT: 11

    // BRANCH: 20
    // BRANCH-NEXT: 21
}
