// `RWByteAddressBuffer.InterlockedCompareExchangeFloatBitwise` and
// `InterlockedCompareStoreFloatBitwise` compare the stored bits with the bits of the comparand.
// The buffer stores `uint` words, so SPIR-V, GLSL and CUDA run an integer compare-exchange on the
// word and reinterpret the values. A stored -0.0 does not match a +0.0 comparand, although the
// two compare equal as floats. The minimum-optimization line checks that the `uint` lowering is
// valid without the optimizer.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-via-glsl -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -xslang -minimum-slang-optimization
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -profile cs_6_6 -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0]):name=buffer
RWByteAddressBuffer buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// SPIRV-COUNT-4: OpAtomicCompareExchange %uint
// GLSL-NOT: GL_EXT_shader_atomic_float
// GLSL-COUNT-4: atomicCompSwap(

[numthreads(1, 1, 1)]
void computeMain()
{
    // Words 1 and 3 hold -0.0.
    buffer.Store(4, 0x80000000u);
    buffer.Store(12, 0x80000000u);

    float original;
    buffer.InterlockedCompareExchangeFloatBitwise(0, 0.0f, 1.0f, original);
    outputBuffer[0] = asuint(original);
    outputBuffer[1] = buffer.Load(0);
    buffer.InterlockedCompareExchangeFloatBitwise(4, 0.0f, 2.0f, original);
    outputBuffer[2] = asuint(original);
    outputBuffer[3] = buffer.Load(4);

    buffer.InterlockedCompareStoreFloatBitwise(8, 0.0f, 3.0f);
    buffer.InterlockedCompareStoreFloatBitwise(12, 0.0f, 4.0f);
    outputBuffer[4] = buffer.Load(8);
    outputBuffer[5] = buffer.Load(12);
}

// 1065353216 is 1.0f, 2147483648 is -0.0f and 1077936128 is 3.0f.
// BUF: 0
// BUF-NEXT: 1065353216
// BUF-NEXT: 2147483648
// BUF-NEXT: 2147483648
// BUF-NEXT: 1077936128
// BUF-NEXT: 2147483648
