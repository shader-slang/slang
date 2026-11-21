// A floating-point atomic compare-exchange on a buffer with a std140 element stride goes through
// an integer view of the buffer with a std430 layout, so the view's word index reaches the same
// bytes as the original element. (The runtime lane emits SPIR-V directly: the GLSL emitter writes
// a structured buffer as std430 whatever its data layout is.)

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[1.0 0.0 0.0 0.0 2.0 0.0 0.0 0.0], stride=16):name=values
RWStructuredBuffer<Atomic<float>, Std140DataLayout> values;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Element 1 is at byte 16, which is word 4 of a view with a 4-byte stride.
// SPIRV-DAG: OpDecorate %_runtimearr_uint ArrayStride 4
// SPIRV-DAG: OpAccessChain %_ptr_StorageBuffer_uint %values{{[_0-9]*}} %int_0 %int_4
// SPIRV-DAG: OpAtomicCompareExchange %uint

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = values[1].compareExchange(2.0f, 5.0f);
    outputBuffer[1] = values[1].load();
    outputBuffer[2] = values[0].load();
}

// BUF: 2
// BUF-NEXT: 5
// BUF-NEXT: 1
