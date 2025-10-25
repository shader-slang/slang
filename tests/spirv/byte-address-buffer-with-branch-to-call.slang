//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV): -vk -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV_BRANCH): -vk -shaderobj -output-using-type -Xslang -DBRANCH

// This tests if the function parameter is properly legalized from ByteAddressBuffer to StructurredBuffer.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11], stride=4):name input
ByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[20 21], stride=4):name input2
ByteAddressBuffer input2;


int getFromBuffer(ByteAddressBuffer buf, int offset)
{
    // When `BRANCH` is not defined, the parameter, `buf`, will be removed and the function body
    // will have a direct access to the global variable, because it can be resolved at the compile-time
    // which global variable should be used.
    //
    // When `BRANCH` is defined, we cannot inline the global variable to the body of the function.
    // The type of parameter must be legalized from ByteAddressBuffer to StructuredBuffer.

    let result = buf.Load<int>(offset);
    return result;
}

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    ByteAddressBuffer buf = input;

#if defined(BRANCH)
    if (tid.x == 0)
        buf = input2;
#endif

    outputBuffer[0] = getFromBuffer(buf, 0);
    outputBuffer[1] = getFromBuffer(buf, 4);

    // SPIRV: 10
    // SPIRV-NEXT: 11

    // SPIRV_BRANCH: 20
    //
    // It seems that there is a bug on the graphics driver
    // when `VariablePointersStorageBuffer` is used.
    // DISABLED_SPIRV_BRANCH-NEXT: 21
}
