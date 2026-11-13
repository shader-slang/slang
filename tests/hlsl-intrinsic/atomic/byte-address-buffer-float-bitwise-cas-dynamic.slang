// `RWByteAddressBuffer.InterlockedCompare{Exchange,Store}FloatBitwise` with run-time comparands and
// values. In byte-address-buffer-float-bitwise-cas.slang every operand is a constant, so the bit
// casts fold away; here they are emitted (`OpBitcast`, `floatBitsToUint`, `uintBitsToFloat`), and
// the returned original value is stored as a `float`.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-via-glsl -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0.0 -0.0 1.5 -2.25], stride=4):name=inputs
RWStructuredBuffer<float> inputs;

//TEST_INPUT:ubuffer(data=[0 0]):name=buffer
RWByteAddressBuffer buffer;

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 0.0 0.0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Word 0 holds 1.5 and word 1 holds -0.0.
    buffer.Store(0, asuint(inputs[2]));
    buffer.Store(4, asuint(inputs[1]));

    float original;
    // 1.5 matches 1.5, so word 0 becomes -2.25.
    buffer.InterlockedCompareExchangeFloatBitwise(0, inputs[2], inputs[3], original);
    outputBuffer[0] = original;
    outputBuffer[1] = asfloat(buffer.Load(0));
    // A +0.0 comparand does not match the stored -0.0.
    buffer.InterlockedCompareExchangeFloatBitwise(4, inputs[0], inputs[2], original);
    outputBuffer[2] = original;
    outputBuffer[3] = asfloat(buffer.Load(4));
    // A -0.0 comparand matches it, so word 1 becomes 1.5.
    buffer.InterlockedCompareStoreFloatBitwise(4, inputs[1], inputs[2]);
    outputBuffer[4] = asfloat(buffer.Load(4));
}

// BUF: 1.5
// BUF-NEXT: -2.25
// BUF-NEXT: -0.0
// BUF-NEXT: -0.0
// BUF-NEXT: 1.5
