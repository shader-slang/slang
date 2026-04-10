//TEST:SIMPLE(filecheck=SPV): -target spirv -O0

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features fp8 -xslang -O0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// SPV-DAG: OpConstant %fp8e4m3 0x1.4p+0
// SPV-DAG: OpConstant %fp8e5m2 0x1.4p+0
// SPV-DAG: OpConstant %bfloat16 0x1.4p+0

// CHECK: 58
// CHECK: 61
// CHECK: 1628

int pack8<T:ICoopElement>(T val)
{
    return bit_cast<uint8_t>(val);
}

int pack16<T:ICoopElement>(T val)
{
    return bit_cast<uint16_t>(val);
}

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = pack8(FloatE4M3(1.25f));
    outputBuffer[1] = pack8(FloatE5M2(1.25f));
    outputBuffer[2] = pack16(BFloat16(1.25f));
}
