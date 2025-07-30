//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -profile spirv_1_3 -capability spvAtomicFloat64MinMaxEXT

// Tests if slang being providing capabilities as a string works as intended

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[require(SPV_EXT_shader_atomic_float_min_max)]
void foo() {}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    foo();
    outputBuffer[0] = 1;
}

//CHECK: 1
