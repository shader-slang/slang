//TEST:COMPARE_COMPUTE(filecheck-buffer=SPIRV): -vk -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV_BRANCH): -target spirv-asm -stage compute -entry computeMain -DBRANCH

// VariablePointersStorageBuffer is not working on the current GCP servers; driver 573.07.
// github issue #8876
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=BRANCH): -vk -shaderobj -output-using-type -Xslang -DBRANCH

// This tests if a dynamic branching works for the legalization from ByteAddressBuffer to StructurredBuffer.

//SPIRV_BRANCH: OpCapability VariablePointersStorageBuffer

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
uniform RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 11], stride=4):name input1
uniform ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[20 21], stride=4):name input2
uniform ByteAddressBuffer input2;

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    ByteAddressBuffer buf = input1;

#if defined(BRANCH)
    if (tid.x == 0)
        buf = input2;
#endif
    // When `BRANCH` is not defined, it is easy to inline the global-param, `input1`, and remove `buf`.
    //
    // When `BRANCH` is defined, we cannot inline the global-param, because it is unknown which one
    // should be inlined until the runtime.
    //
    // From the IR perspecitve, the merging-point will take an IRParam and its type needs to be
    // legalized from ByteAddressBuffer to StructuredBuffer.
    // The IR dump will look something like the following:
    // ```
    //     ifElse(%22, %23, %24, %25)
    //   block %24:
    //     unconditionalBranch(%25, %input1)
    //   block %23:
    //     unconditionalBranch(%25, %input2)
    //   block %25(
    //       [nameHint("buf")]
    //       param %buf      : ByteAddressBuffer # <== Need to be legalized
    //       ):
    //   let %26 : Int = byteAddressBufferLoad(%buf, 0 : UInt, 0 : UInt)
    // ```

    let result0 = buf.Load<int>(0);
    let result1 = buf.Load<int>(4);

    outputBuffer[0] = result0;
    outputBuffer[1] = result1;

    // SPIRV: 10
    // SPIRV-NEXT: 11

    // BRANCH: 20
    // BRANCH-NEXT: 21
}
