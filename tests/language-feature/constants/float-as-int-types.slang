//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -dx12 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -mtl -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4, count=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test __floatAsInt with half, float, and double types
// Each returns the appropriate integer type based on input

// Half (16-bit float) -> int16_t
// half 1.0 = 0x3C00 = 15360
static const int16_t halfOne = __floatAsInt(1.0h);

// Float (32-bit) -> int32_t  
// float 1.0 = 0x3F800000 = 1065353216
static const int floatOne = __floatAsInt(1.0f);

// Double (64-bit) -> int64_t
// double 1.0 = 0x3FF0000000000000 = 4607182418800017408
static const int64_t doubleOne = __floatAsInt(1.0lf);

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Half 1.0h bit pattern: 0x3C00 = 15360
    // BUF: 15360
    outputBuffer[0] = int(halfOne);

    // Float 1.0f bit pattern: 0x3F800000 = 1065353216
    // BUF-NEXT: 1065353216
    outputBuffer[1] = floatOne;

    // Double 1.0lf lower 32 bits: 0x00000000 = 0
    // BUF-NEXT: 0
    outputBuffer[2] = int(doubleOne & 0xFFFFFFFF);

    // Double 1.0lf upper 32 bits: 0x3FF00000 = 1072693248
    // BUF-NEXT: 1072693248
    outputBuffer[3] = int(doubleOne >> 32);
}
