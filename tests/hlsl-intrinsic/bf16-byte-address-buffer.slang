//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features bfloat16 -skip-spirv-validation
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features bfloat16

// Test BFloat16 load/store through RWByteAddressBuffer to verify
// the byte-address legalization path (kIROp_BFloat16Type → UInt16 mapping).

// CHECK: 30
// CHECK: 50
// CHECK: 80

//TEST_INPUT:ubuffer(data=[0x00004040 0 0 0], stride=4):name byteBuffer
RWByteAddressBuffer byteBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // Load BFloat16 from byte offset 0
    // 0x4040 in BFloat16 = 3.0
    BFloat16 val = byteBuffer.Load<BFloat16>(0);
    outputBuffer[0] = int(float(val) * 10.0);

    // Store a new value and load it back
    BFloat16 newVal = BFloat16(5.0);
    byteBuffer.Store<BFloat16>(2, newVal);
    BFloat16 loaded = byteBuffer.Load<BFloat16>(2);
    outputBuffer[1] = int(float(loaded) * 10.0);

    // Arithmetic on loaded value
    BFloat16 sum = val + newVal;
    byteBuffer.Store<BFloat16>(4, sum);
    BFloat16 loadedSum = byteBuffer.Load<BFloat16>(4);
    outputBuffer[2] = int(float(loadedSum) * 10.0);
}
