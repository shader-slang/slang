//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV): -vk -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV_BRANCH): -vk -shaderobj -output-using-type -Xslang -DBRANCH

// This tests if a dynamic branching works for the legalization from ByteAddressBuffer to StructurredBuffer.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11], stride=4):name input
StructuredBuffer<int> input;

//TEST_INPUT:ubuffer(data=[20 21], stride=4):name input2
StructuredBuffer<int> input2;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    StructuredBuffer<int> buf = input;

#if defined(BRANCH)
    if (tid.x == 0)
        buf = input2;
#endif
    // When `BRANCH` is not defined, it is easy to inline the global-param, `input`, and remove `buf`.
    //
    // When `BRANCH` is defined, we cannot inline the global-param, because it is unknown which one
    // should be inlined until the runtime.

    outputBuffer[0] = buf[0];
    outputBuffer[1] = buf[1];

    // SPIRV: 10
    // SPIRV-NEXT: 11

    // SPIRV_BRANCH: 20
    //
    // It seems that there is a bug on the graphics driver
    // when `VariablePointersStorageBuffer` is used.
    // DISABLE_SPIRV_BRANCH-NEXT: 21
}
